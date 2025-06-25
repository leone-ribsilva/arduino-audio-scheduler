# Checklist de Conexão por Componente

## 🔌 DFPlayer Mini

| DFPlayer Pin  | Conectar a              | Observações                              |
| ------------- | ----------------------- | ---------------------------------------- |
| VCC           | 5V do Arduino           | Pode usar 3.3V também, mas 5V é ideal    |
| GND           | GND do Arduino          | Terra comum                              |
| TX            | Pino 10 do Arduino      | Comunicação serial (RX Arduino)          |
| RX            | Pino 11 do Arduino      | Comunicação serial (TX Arduino)          |
| SPK_1 / SPK_2 | Alto-falante (opcional) | Ou use saída via P2/RCA via amplificador |

> 💡 Se for usar saída de áudio via P2/RCA para mesa de som, conecte nos pinos DAC_R e DAC_L do DFPlayer (e GND), ligados a um conector P2 ou RCA com divisor resistivo se necessário.

---

## ⏱️ RTC DS3231

| RTC Pin | Conectar a        | Observações                 |
| ------- | ----------------- | --------------------------- |
| VCC     | 5V do Arduino     | Ou 3.3V se for um módulo 3V |
| GND     | GND do Arduino    | Terra comum                 |
| SDA     | A4 no Arduino Uno | Comunicação I2C (dados)     |
| SCL     | A5 no Arduino Uno | Comunicação I2C (clock)     |

---

## 💾 Cartão microSD (caso separado do DFPlayer)

| SD Pin | Conectar a         | Observações                               |
| ------ | ------------------ | ----------------------------------------- |
| VCC    | 5V do Arduino      | Pode exigir divisor de tensão se for 3.3V |
| GND    | GND do Arduino     | Terra comum                               |
| CS     | Pino 4 do Arduino  | Usado no código como `SD_CS_PIN`          |
| MOSI   | Pino 11 do Arduino | Comunicação SPI                           |
| MISO   | Pino 12 do Arduino | Comunicação SPI                           |
| SCK    | Pino 13 do Arduino | Comunicação SPI                           |

---

## 🔧 Conexão com Resistência (se necessário)

-   Entre DFPlayer RX e pino 11 do Arduino: usar divisor resistivo (ex: 1k + 2k resistores) para evitar sobrecarga no pino do DFPlayer (pois ele opera a 3.3V e o Arduino a 5V).
-   Se for ligar em amplificador externo ou entrada de mesa via P2/RCA, pode usar resistores de 1kΩ para cada saída L/R, somando com GND no jack.

![Diagrama de ligação](diagrama_ligacao.png)
