
PASSO A PASSO PARA DEPLOY NO FIREBASE HOSTING

1. Instale o Firebase CLI (caso ainda não tenha):
   npm install -g firebase-tools

2. Faça login:
   firebase login

3. Extraia todos os arquivos desta pasta em um diretório local
   Ex: mkdir alimentador-site && cd alimentador-site
   Copie os arquivos index.html, style.css, script.js, firebase.json e README.txt para lá

4. Inicialize o projeto:
   firebase init
   - Escolha "Hosting"
   - Selecione o projeto criado no Firebase Console
   - Use "." como diretório público
   - Diga NÃO para SPA
   - NÃO sobrescreva index.html

5. Faça o deploy:
   firebase deploy

O terminal mostrará um link com HTTPS para acessar sua página.
