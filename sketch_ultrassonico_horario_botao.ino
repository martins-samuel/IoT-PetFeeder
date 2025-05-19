#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Stepper.h>
#include <NewPing.h>

// ======= CONFIGURAÇÃO DO WIFI =======
const char* ssid = "nomedarede";
const char* password = "senhadarede";

// ======= USUÁRIO E SENHA PARA ACESSO =======
String username = "admin";
String senha = "1234";

// ======= CONFIGURAÇÃO DO MOTOR =======
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
Stepper motor(2048, IN1, IN3, IN2, IN4);

// ======= CONFIGURAÇÃO DO SENSOR ULTRASSÔNICO =======
#define TRIGGER_PIN D5
#define ECHO_PIN D6
#define DISTANCIA_MAXIMA_CM 30
#define LIMIAR_QUASE_VAZIO_CM 8
NewPing sonar(TRIGGER_PIN, ECHO_PIN, DISTANCIA_MAXIMA_CM);

// ======= SERVIDOR E NTP =======
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000);  // Horário de Brasília

// ======= HORÁRIOS E CONTROLE =======
String hora1 = "08:00";
String hora2 = "18:00";
bool executadoHoje[2] = {false, false};
bool alertaVazio = false;

unsigned long ultimaAtualizacaoNivel = 0;
unsigned long intervaloAtualizacaoNivel = 6UL * 60 * 60 * 1000; // 6 horas em ms

long nivelRacaoAtual = -1;  // valor da última medição válida

// ======= HTML DINÂMICO =======
String gerarPagina() {
  String nivel = (nivelRacaoAtual <= 0) ? "Indisponível" : String(nivelRacaoAtual) + " cm";
  String alerta = (nivelRacaoAtual > 0 && nivelRacaoAtual < LIMIAR_QUASE_VAZIO_CM) ? "<p style='color:red;'>⚠️ Ração quase vazia!</p>" : "";

  String pagina = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <title>Alimentador de Gatos</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      background-color: #f4f4f4;
      font-family: Arial, sans-serif;
      text-align: center;
      padding-top: 50px;
    }
    h1 {
      color: #333;
    }
    button {
      background-color: #4CAF50;
      color: white;
      padding: 12px 24px;
      font-size: 16px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
    }
    button:hover {
      background-color: #45a049;
    }
    input[type=time] {
      padding: 10px;
      font-size: 16px;
      margin: 10px;
    }
  </style>
</head>
<body>
  <h1>Alimentador de Pets</h1>
  <form action="/girar" method="POST">
    <button type="submit">Liberar Ração</button>
  </form>
  <br>
  <h2>Agendar Alimentação</h2>
  <form action="/agendar" method="POST">
    <label>Horário 1: <input type="time" name="hora1" required></label><br>
    <label>Horário 2: <input type="time" name="hora2" required></label><br><br>
    <button type="submit">Salvar Horários</button>
  </form>
  <p>Horários atuais: %HORARIOS%</p>
  <p>Nível atual de ração: %NIVEL%</p>
  %ALERTA%
</body>
</html>
)rawliteral";

  pagina.replace("%HORARIOS%", hora1 + " e " + hora2);
  pagina.replace("%NIVEL%", nivel);
  pagina.replace("%ALERTA%", alerta);
  return pagina;
}

// ======= AUTENTICAÇÃO =======
void autenticar() {
  if (!server.authenticate(username.c_str(), senha.c_str())) {
    return server.requestAuthentication();
  }
}

// ======= FUNÇÃO PARA LIBERAR RAÇÃO =======
void alimentar() {
  Serial.println("[LOG] Liberando ração...");
  motor.step(2048); // 1 volta
  Serial.println("[LOG] Alimentação realizada com sucesso.");
}

// ======= FUNÇÃO DE ATUALIZAÇÃO DE NÍVEL =======
void atualizarNivelRacao() {
  long medida = sonar.ping_cm();
  if (medida > 0) {
    nivelRacaoAtual = medida;
    Serial.print("[LOG] Nível de ração atualizado: ");
    Serial.print(nivelRacaoAtual);
    Serial.println(" cm");
  } else {
    Serial.println("[LOG] Medida inválida ou sensor falhou.");
  }
}

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  motor.setSpeed(10);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado. IP: " + WiFi.localIP().toString());

  timeClient.begin();
  atualizarNivelRacao(); // primeira medição
  ultimaAtualizacaoNivel = millis();

  server.on("/", []() {
    autenticar();
    server.send(200, "text/html", gerarPagina());
  });

  server.on("/girar", HTTP_POST, []() {
    autenticar();
    alimentar();
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/agendar", HTTP_POST, []() {
    autenticar();
    if (server.hasArg("hora1") && server.hasArg("hora2")) {
      hora1 = server.arg("hora1");
      hora2 = server.arg("hora2");
      Serial.println("[LOG] Horários atualizados: " + hora1 + " e " + hora2);
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.begin();
  Serial.println("Servidor iniciado.");
}

// ======= LOOP PRINCIPAL =======
void loop() {
  server.handleClient();
  timeClient.update();

  // Verifica se passaram 6h desde a última atualização do nível
  if (millis() - ultimaAtualizacaoNivel >= intervaloAtualizacaoNivel) {
    atualizarNivelRacao();
    ultimaAtualizacaoNivel = millis();
  }

  // Alimentação automática
  int horaAtual = timeClient.getHours();
  int minutoAtual = timeClient.getMinutes();

  String horaStr = (horaAtual < 10 ? "0" : "") + String(horaAtual) + ":" +
                   (minutoAtual < 10 ? "0" : "") + String(minutoAtual);

  String agendados[] = {hora1, hora2};

  for (int i = 0; i < 2; i++) {
    if (horaStr == agendados[i]) {
      if (!executadoHoje[i]) {
        alimentar();
        executadoHoje[i] = true;
      }
    } else {
      executadoHoje[i] = false;
    }
  }
}
