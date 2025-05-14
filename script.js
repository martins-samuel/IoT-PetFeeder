
// ⚠️ Substitua pelos dados reais do seu Firebase
const firebaseConfig = {
  apiKey: "SUA_API_KEY",
  authDomain: "SEU_DOMINIO.firebaseapp.com",
  databaseURL: "https://SEU_DOMINIO.firebaseio.com",
  projectId: "SEU_ID",
  storageBucket: "SEU_ID.appspot.com",
  messagingSenderId: "SEU_ID",
  appId: "SUA_APP_ID"
};

// Inicializa o Firebase
firebase.initializeApp(firebaseConfig);
const db = firebase.database();

// Função acionada ao clicar no botão
function liberar() {
  db.ref("comando").set("liberar")
    .then(() => alert("Comando enviado!"))
    .catch(err => alert("Erro ao enviar comando: " + err));
}
