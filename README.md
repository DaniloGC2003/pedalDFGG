# Formato dos pacotes na comunicação serial Arduino-ESP32

Os pacotes seriais são compostos de um header seguido de 8 bytes: **"ESP" + message_id bytes**
Os códigos de mensagem atualmente implementados são:
- UPDATE_ENCODER_MESSAGE_ID 11. Mensagem para atualização de um dos parâmetros dos encoders/controles deslizantes. A mensagem consiste em 2 bytes: o primeiro representa o parâmetro a ser atualizado; o segundo representa o valor a ser configurado no parâmetro.
- UPDATE_ALL_ENCODERS_MESSAGE_ID 12. Mensagem para atualização de todos os parâmetros dos encoders/controles deslizantes. A mensagem consiste em 4 bytes. Cada um deles representa o valor de um parâmetro diferente.

## ESP32 -> Arduino
- Uma mensagem da ESP32 para o Arduino é sempre enviada quando a ESP32 recebe uma mensagem de atualização de parâmetros dos encoders (UPDATE_ENCODER_MESSAGE_ID 11). Abaixo, um exemplo de mensagem de atualização de parâmetros do encoder 2:

    Byte 0: 0x45 (69)\
    Byte 1: 0x53 (83)\
    Byte 2: 0x50 (80)\
    Byte 3: 0x0B (11)\
    Byte 4: 0x02 (2)\
    Byte 5: 0x1D (29)\
    Byte 6: 0x00 (0)\
    Byte 7: 0x00 (0)\
    Byte 8: 0x00 (0)\
    Byte 9: 0x00 (0)\
    Byte 10: 0x00 (0)

- A ESP32 também pode enviar ao Arduino mensagens com o código UPDATE_ALL_ENCODERS_MESSAGE_ID. Nesse caso, o Arduino é encarregado de enviar à ESP32 todos os atuais valores referentes aos parâmetros de cada encoder.

## Arduino -> ESP32
- Uma mensagem de ID UPDATE_ENCODER_MESSAGE_ID conterá o ID e o valor atual referente a um dos parâmetros dos encoders do Arduino. A ESP32, por sua vez, deverá repassar essa informação à página web, a fim de que os valores sejam atualizados ao usuário.

- Uma mensagem de ID UPDATE_ALL_ENCODERS_MESSAGE_ID do Arduino para a ESP32 é uma resposta do Arduino a uma mensagem anteriormente enviada com o mesmo ID. Essa mensagem recebida pela ESP32 contém todos os valores atuais dos encoders. Esses valores serão repassados via BLE para que a página seja atualizada.