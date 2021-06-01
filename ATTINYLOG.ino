#define LEVEL 1500
#define MAXLEVEL 1800
#define TRIG A2
#define VOUT A6

#define USART_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)  
#define USARTPA1
#define SERIALBAUD 250000
uint16_t val;
uint8_t cnt;

void setup() {
  usartInit();
  VREF.CTRLA = 0x1<<4;//6-4:ADCREF,2-0:DACREF,(1:1.1v, 2:2.5V, 3:4.3V, 4:1.5V)
  ADC0.MUXPOS  = 0x1E;//4-0:MUXPOS(0:AIN0,,7:AIN7,,1E:TEMSENSE)
  ADC0.CTRLC   = 0x04;//6:SAMPCAP(1:vref>1v,0:vref<1v), 5-4:REFSEL(INTERNAL REF, VDD, EXTERNAL REF, -), 2-0:PRESC(0:div2, 7:dev256)
  ADC0.CALIB   = 0x01;//0:DUTYCYC(0:50% ADCCLK>1.5MHz,1:25% < 1.5MHz)
  ADC0.CTRLA   = 0x03;//7:RUNSTDBY, 2:RESSEL(0:10bit, 1:8bit), 1:FREERUN, 0:ENABLE
  ADC0.COMMAND = 0x01;//0:STCONV  
}
void loop() {  
  int8_t sigrow_offset = SIGROW.TEMPSENSE1; // 識票列から符号付き変位(ｵﾌｾｯﾄ)補正値読み込み
  uint8_t sigrow_gain = SIGROW.TEMPSENSE0; // 識票列から符号なし利得補正値読み込み
  uint16_t adc_reading = ADC0.RES; // 1.1V内部基準電圧でのA/D変換結果
  uint32_t temp = adc_reading - sigrow_offset;
  temp *= sigrow_gain; // 結果(10ﾋﾞｯﾄ×8ﾋﾞｯﾄ)は16ﾋﾞｯﾄ変数を溢れるかもしれません。
  //temp += 0x80; // 次の除算で正しい丸めを得るために1/2を加算
  uint16_t temperature_in_K = temp >> 8; // ｹﾙﾋﾞﾝ温度を得るために結果を除算
  temp &= 0xff;
  temp *= 100;
  temp /= 256;
  usartSendChar('\n');
  usartSendUint(adc_reading,4,10);
  usartSendString(",");
  usartSendUint(temperature_in_K,4,10);
  usartSendString(".");
  usartSendUint(temp,2,10);
  delay(1000);
}

/******User Serial Send*******/
#define USART_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)  
void usartInit() {
  USART0.BAUD = (uint16_t)USART_BAUD_RATE(SERIALBAUD);
  USART0.CTRLB |= USART_TXEN_bm;
#ifdef USARTPA1
  PORTMUX.CTRLB |= 1;
  PORTA.DIR |= PIN1_bm;
#else
  PORTA.DIR |= PIN6_bm;
#endif
}

void usartSendChar(char c) {
  while (!(USART0.STATUS & USART_DREIF_bm)){;}
  USART0.TXDATAL = c;    
}
void usartSendString(char *str) {
 for(size_t i = 0; i < strlen(str); i++)
 {
   usartSendChar(str[i]);
 }
}
/// digit=0: zero suppress, 1~16: fixed digit
void usartSendUint(uint16_t d, char digit, char base) {
  char i,b[16]={0};
  if((digit<0)||(digit>16)) return;
  if((base<2)||(base>36)) return;
  for(i=0; i<16; i++) {
    b[i]=d % base;
    d = d / base;
    if(d==0) break;
  }
  if(digit>0) i=digit-1;
  for(;i>=0; i--) {
    if(b[i]>9) { b[i] += 'A' - '0' - 10; }
    usartSendChar(b[i]+'0');  
  }
}

void dump_hex(const char *msg, const uint8_t *row, int n, bool ascii)
{
    usartSendString(msg);
    for (int i = 0; i < n; i++) {
      usartSendChar(' ');
      usartSendUint( row[i],2,16);
    }
    if (ascii) {
        char buf[n + 1];
        memcpy(buf, row, n);
        buf[n] = '\0';
        usartSendChar(' ');
        usartSendString(buf);
    }
    usartSendChar('\n');
}
