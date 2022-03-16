// LOG output library for small footprint like ATTINY202
// 2022/3/16 H.Nii

// for Serial port setting
#define USARTPA1
#define SERIALBAUD 9600

void setup() {
  usartInit();
}
void loop() {  
  int test = 10;
  usartSendString("Decimal 10 (normal)= ");
  usartSendUint(test,0,10);
  usartSendString("\n Decimal 10 (4 digit)= ");
  usartSendUint(test,4,10);
  usartSendString("\n Hex 10 (2 digit)= ");
  usartSendUint(test,2,16);
  usartSendChar('\n');
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
