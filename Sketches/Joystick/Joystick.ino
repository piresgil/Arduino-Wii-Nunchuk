#include <Wire.h>
#include <Joystick.h>

/**
 * @brief Objeto da biblioteca Joystick para controlar o joystick virtual.
 */
Joystick_ Joystick;

/**
 * @brief Endereço I2C do Nunchuck.
 */
const int nunchuckAddress = 0x52;

/**
 * @brief Array para armazenar os 6 bytes de dados lidos do Nunchuck.
 *
 * Os bytes contêm informações sobre a posição do joystick, acelerômetros e botões.
 */
uint8_t nunchuckData[6];

/**
 * @brief Inicializa o Nunchuck enviando sequências de inicialização específicas.
 *
 * Esta função envia os comandos necessários para desativar a criptografia
 * e configurar o Nunchuck para fornecer dados brutos.
 */
void nunchuckInit() {
  Wire.beginTransmission(nunchuckAddress);
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();

  Wire.beginTransmission(nunchuckAddress);
  Wire.write(0xFB);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(100);
}

/**
 * @brief Solicita os dados do Nunchuck através da interface I2C.
 *
 * Envia um byte '0x00' para o endereço do Nunchuck para solicitar
 * o envio dos 6 bytes de dados.
 */
void nunchuckRequestData() {
  Wire.beginTransmission(nunchuckAddress);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
}

/**
 * @brief Lê os 6 bytes de dados enviados pelo Nunchuck.
 *
 * Os dados lidos são armazenados no array `nunchuckData`.
 * É importante chamar `nunchuckRequestData()` antes de chamar esta função.
 */
void nunchuckReadData() {
  Wire.requestFrom(nunchuckAddress, 6);
  for (int i = 0; i < 6; i++) {
    nunchuckData[i] = Wire.read();
  }
}

/**
 * @brief Função de configuração que é executada uma vez no início do programa.
 *
 * Inicializa a comunicação I2C (Wire) e a funcionalidade do joystick virtual.
 * Também chama a função `nunchuckInit()` para inicializar o Nunchuck.
 */
void setup() {
  Wire.begin();
  Joystick.begin();
  nunchuckInit();
}

/**
 * @brief Função principal de loop que é executada continuamente após a função de configuração.
 *
 * Solicita e lê os dados do Nunchuck, mapeia os valores do joystick para a faixa do joystick virtual,
 * lê o estado dos botões Z e C, define os eixos e botões do joystick virtual e envia o estado.
 */
void loop() {
  nunchuckRequestData();
  nunchuckReadData();

  // Extrai os valores dos eixos X e Y do joystick dos dados lidos.
  int joyX = nunchuckData[0];
  int joyY = nunchuckData[1];

  // Extrai o estado dos botões Z e C dos dados lidos.
  // O botão Z está no bit 0, e o botão C no bit 1 do sexto byte.
  bool zButton = !(nunchuckData[5] & 0x01);
  bool cButton = !((nunchuckData[5] & 0x02) >> 1);

  // Mapeia os valores do joystick (0-255) para a faixa do joystick virtual (0-1023).
  int mappedX = map(joyX, 0, 255, 0, 1023);
  int mappedY = map(joyY, 0, 255, 1023, 0);  // Eixo Y invertido para corresponder à direção intuitiva.

  // Define os valores dos eixos X e Y do joystick virtual.
  Joystick.setXAxis(mappedX);
  Joystick.setYAxis(mappedY);
  // Define o estado dos botões do joystick virtual (botão 1 para Z, botão 2 para C).
  Joystick.setButton(0, zButton); // Botão 1
  Joystick.setButton(1, cButton); // Botão 2

  // Envia o estado atual do joystick virtual para o computador.
  Joystick.sendState();
  delay(20); // Pequeno atraso para evitar sobrecarga de comunicação.
}