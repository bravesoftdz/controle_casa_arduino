#include <WProgram.h>
#include <Client.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Server.h>
#include <IRremote.h>


  //parametros web_ethernet
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  IPAddress ip(192,168,1,146);
  EthernetServer server(8090);
  int conta_caracter;
  String linha_de_entrada;
  
  //declarando os pinos para cada função  
  const int IR = 9; //LED InfraVermelho
  const int lampada1 = 13;
  //int lampada2 = A11;
  
  //sensor de temperatura
  int pinoSensor = A8; //pino que está ligado o terminal central do LM35 (porta analogica 0)
  int valorLido_temp = 0; //valor lido na entrada analogica
  float temperatura = 0; //valorLido convertido para temperatura
  IRsend irsend;
  
  // Storage for the recorded code
  unsigned int rawCodes[RAWBUF]; // The durations if raw
  int codeLen; // The length of the code
  int toggle = 0; // The RC5/6 toggle state
  
void setup() {
  Ethernet.begin(mac, ip); // inicia ethernet shield  
  server.begin();
  Serial.print("Servidor no Ar");
  Serial.begin(9600); // RFID reader SOUT pin connected to Serial RX pin at 2400bps  
 
  // Temperatura
  valorLido_temp = analogRead(pinoSensor);
  temperatura = (valorLido_temp * 0.00488);  // 5V / 1023 = 0.00488 (precisão do A/D)
  temperatura = temperatura * 100; //Conv
  
  pinMode(IR, OUTPUT);
  
  pinMode(lampada1, OUTPUT);
  digitalWrite(lampada1, LOW);
  
  /*pinMode(lampada2, OUTPUT);
  digitalWrite(lampada2, HIGH);*/ 
}
  
void loop(){
  //ir.available();
  //capturaCodigo = ir.read() ; 
  //Serial.println(capturaCodigo); 
  /*
  Verificar qual é o código do controle e fazer os IFs
  */
  
  // parametros web client
  EthernetClient client = server.available();
  if (client) {
      // an http request ends with a blank line
      boolean current_line_is_blank = true;
      conta_caracter=0 ;
      linha_de_entrada="" ;
      
      while (client.connected()) {
        if (client) {
          // recebe um caracter enviado pelo browser
          char c = client.read();
          // se a linha não chegou ao máximo do armazenamento
          // então adiciona a linha de entrada
          if(linha_de_entrada.length() < 10) {
            linha_de_entrada.concat(c) ;
          }
   
          // Se foi recebido um caracter linefeed - LF
          // e a linha está em branco , a requisição http encerrou.
          // Assim é possivel iniciar o envio da resposta
          
          if (c == '\n' && current_line_is_blank) {
            // envia uma resposta padrão ao header http recebido
            /*client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            // começa a enviar o formulário
            client.print("<html>") ;
            client.print("<body>");
            
            client.println("<br>") ;
            client.println("<h2>CONTROLE DO LED</h2><hr/>");
            client.println("<form method=get name=LED>") ;
            
            client.println("LIGA <input ") ;
            // verifica o status do led e ativa o radio button
            // correspondente
            if(LEDON) {
              client.println("checked='checked'") ;
            }
            client.println("name='LED' value='1' type='radio' >");
            
            client.println("DESLIGA <input ") ;
            if(!LEDON) {
              client.println("checked='checked'") ;
            }
            client.println("name='LED' value='0' type='radio' >");
            // exibe o botão do formulário
            client.println("<br><br><br><input type=submit value='ATUALIZA'></form>") ;
            client.println("<br><font color='blue' size='3'>Acesse <a href=http://www.arduino4fun.wordpress.com/>www.arduino4fun.wordpress.com</a>");
            client.println("</body>") ;
            client.println("</html>");*/
  
            break;
          }
          
          if (c == '\n') {
            // se o caracter recebido é um linefeed então estamos começando a receber uma
            // nova linha de caracteres
            // os codigos de impressão abaixo são para depuração e visualizar no monitor serial
            // o que está chegando do browser
            Serial.print(linha_de_entrada.length()) ;
            Serial.print("->") ;
            Serial.print(linha_de_entrada) ;
            // Analise aqui o conteudo enviado pelo submit
            if(linha_de_entrada.indexOf("GET") != -1 ){// se a linha recebida contem GET e LED=ON então guarde o status da lampada              
              // Liga a lampada1
              if(linha_de_entrada.indexOf("CMD=L1ON") != -1 ){
                lampada1_on() ;
               }
              if(linha_de_entrada.indexOf("CMD=L1OFF") != -1 ){
              // se a linha recebida contem GET e LED=OFF então guarde o status do led
                lampada1_off();
              }
              
              
              /******************************************** TELEVISAO SONY ***********************************************/
              if ((linha_de_entrada.indexOf("CMD=LIGARTV") != -1) or (linha_de_entrada.indexOf("CMD=DESLIGARTV") != -1 )){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xa90); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //Canal mais
              if (linha_de_entrada.indexOf("CMD=CANALMAIS") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x90); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //Canal Menos
              if (linha_de_entrada.indexOf("CMD=CANALMENOS") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x890); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //Volume mais
              if (linha_de_entrada.indexOf("CMD=VOLUMEMAIS") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x490); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //Volume menos
              if (linha_de_entrada.indexOf("CMD=VOLUMEMENOS") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xc90); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }              
              //OK
              if (linha_de_entrada.indexOf("CMD=OK") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xa70); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //Direta
              if (linha_de_entrada.indexOf("CMD=DIREITATV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xcd0); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //Esquerda
              if (linha_de_entrada.indexOf("CMD=ESQUERDATV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x2d0); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //cima
              if (linha_de_entrada.indexOf("CMD=CIMATV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x2f0); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //baixo
              if (linha_de_entrada.indexOf("CMD=BAIXOTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xaf0); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }              
              //voltar
              if (linha_de_entrada.indexOf("CMD=VOLTARTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x62e9); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }              
              //1
              if (linha_de_entrada.indexOf("CMD=UMTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x10); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }              
              //2
              if (linha_de_entrada.indexOf("CMD=DOISTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x810); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }              
              //3
              if (linha_de_entrada.indexOf("CMD=TRESTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x410); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //4
              if (linha_de_entrada.indexOf("CMD=QUATROTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xc10); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //5
              if (linha_de_entrada.indexOf("CMD=CINCOTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x210); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //6
              if (linha_de_entrada.indexOf("CMD=SEISTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xa10); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //7
              if (linha_de_entrada.indexOf("CMD=SETETV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x610); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //8
              if (linha_de_entrada.indexOf("CMD=OITOTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xE10); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //9
              if (linha_de_entrada.indexOf("CMD=NOVETV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x110); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //0
              if (linha_de_entrada.indexOf("CMD=ZEROTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x910); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //PONTO
              if (linha_de_entrada.indexOf("CMD=PONTOTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x5CE9); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }              
              //HOME
              if (linha_de_entrada.indexOf("CMD=HOMETV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x70); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //OPCOES
              if (linha_de_entrada.indexOf("CMD=OPCAOTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0x36E9); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              //INPUT
              if (linha_de_entrada.indexOf("CMD=INPUTTV") != -1){
                for (int i = 0; i < 3; i++) {
                  sendCode(0, 2, 0xA50); // Sony TV power code
                  delay(40);
                }
                delay(50);
              }
              
              /******************************************** FIM TELEVISAO SONY ********************************************/
              
              
              /*if(linha_de_entrada.indexOf("CMD=L2ON") != -1 ){
                lampada2_on() ;
               }
              if(linha_de_entrada.indexOf("CMD=L2OFF") != -1 ){
              // se a linha recebida contem GET e LED=OFF então guarde o status do led
                lampada2_off() ;
              }
              if(linha_de_entrada.indexOf("CMD=ALARMEON") != -1 ){
                alarme_on();
               }
              if(linha_de_entrada.indexOf("CMD=ALARMEOFF") != -1 ){
              // se a linha recebida contem GET e LED=OFF então guarde o status do led
                alarme_off();
              }
              if(linha_de_entrada.indexOf("CMD=TEMP") != -1 ){
              // se a linha recebida contem GET e LED=OFF então guarde o status do led
                temp_atual();
              }*/
            }             
                       
            current_line_is_blank = true;
            linha_de_entrada="" ;
            
          } else if (c != '\r') {
            // recebemos um carater que não é linefeed ou retorno de carro
            // então recebemos um caracter e a linha de entrada não está mais vazia
            current_line_is_blank = false;
          }
        }
      }
      // dá um tempo para o browser receber os caracteres
      delay(10);
      client.stop();
    }// fim parametros web client  
}

void sendCode(int repeat, int codeType, unsigned long codeValue) {
  if (codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(codeValue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      toggle = 1 - toggle;
    }
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    } 
    else {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}
  
void lampada1_on(){
  digitalWrite(lampada1, HIGH);
}

void lampada1_off(){
  digitalWrite(lampada1, LOW);
}
