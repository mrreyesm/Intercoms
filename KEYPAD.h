//4x4 Keypad

#define KBR1			LATBbits.LATB3		// Row 1    //outputs
#define KBR2			LATBbits.LATB4		// Row 2
#define KBR3			LATBbits.LATB5	    // Row 3
#define KBR4			LATDbits.LATD7		// Row 4

#define KBC1			PORTBbits.RB0		// Colomn 1 //inputs
#define KBC2			PORTBbits.RB1		// Colomn 2
#define KBC3			PORTBbits.RB2		// Colomn 3
//#define KBC4			LATBbits.LATB0		// Colomn 4

unsigned char KBDrd(void); 
unsigned char KBDwait(void);
unsigned char KBDrdDoor(void);
unsigned char KBDwaitDoor(void);
unsigned char KBDwait100ms(void);

#define  SENSPUERTA  PORTCbits.RC0

unsigned char KBDrd(void)
{
	//start the scanning process
	KBR1 = 0;		// scan keypress on 1st row: 1, 2, 3, A
	KBR2 = 1;
	KBR3 = 1;
	KBR4 = 1;
	__delay_ms(5);
	if (KBC1 == 0)	return '1';		// Key '1' is pressed
	if (KBC2 == 0)  return '2';		// Key '2' is pressed
	if (KBC3 == 0)  return '3';		// Key '3' is pressed
	//if (KBC4 == 0) return 'A';		// Key 'A' is pressed, we will store as 10

	KBR1 = 1;		// scan keypress on 2nd row: 4, 5, 6, B
	KBR2 = 0;
	KBR3 = 1;
	KBR4 = 1;
	__delay_ms(5);
	if (KBC1 == 0)	return '4';		// Key '4' is pressed
	if (KBC2 == 0)  return '5';		// Key '5' is pressed
	if (KBC3 == 0)  return '6';		// Key '6' is pressed
	//if (KBC4 == 0) return 'B';		// Key 'B' is pressed, we will store as 11

	KBR1 = 1;		// scan keypress on 3rd row: 7, 8, 9, C
	KBR2 = 1;
	KBR3 = 0;
	KBR4 = 1;
	__delay_ms(5);
	if (KBC1 == 0)	return '7';		// Key '7' is pressed
	if (KBC2 == 0)  return '8';		// Key '8' is pressed
	if (KBC3 == 0)  return '9';		// Key '9' is pressed
    //(if (KBC4 == 0) return 'C';		// Key 'C' is pressed, we will store as 12

	KBR1 = 1;		// scan keypress on 4th row: *, 0, #, D
	KBR2 = 1;
	KBR3 = 1;
	KBR4 = 0;
	__delay_ms(5);
	if (KBC1 == 0)	return '*';		// Key '*' is pressed, we will store as 14
	if (KBC2 == 0)  return '0';		// Key '0' is pressed
	if (KBC3 == 0)  return '#';		// Key '#' is pressed, we will store as 15
	//if (KBC4 == 0) return 'D';		// Key 'D' is pressed, we will store as 13

	return 0xFF;					// if no key press, the register is 0xFF
}

unsigned char KBDrdDoor(void)
{
	//start the scanning process
	KBR1 = 0;		// scan keypress on 1st row: 1, 2, 3, A
	KBR2 = 1;
	KBR3 = 1;
	KBR4 = 1;
	__delay_ms(5);
	if (KBC1 == 0)	return '1';		// Key '1' is pressed
	if (KBC2 == 0)  return '2';		// Key '2' is pressed
	if (KBC3 == 0)  return '3';		// Key '3' is pressed
    if (SENSPUERTA == 0)  return 'A';
	//if (KBC4 == 0) return 'A';		// Key 'A' is pressed, we will store as 10

	KBR1 = 1;		// scan keypress on 2nd row: 4, 5, 6, B
	KBR2 = 0;
	KBR3 = 1;
	KBR4 = 1;
	__delay_ms(5);
	if (KBC1 == 0)	return '4';		// Key '4' is pressed
	if (KBC2 == 0)  return '5';		// Key '5' is pressed
	if (KBC3 == 0)  return '6';		// Key '6' is pressed
    if (SENSPUERTA == 0)  return 'A';	
    //if (KBC4 == 0) return 'B';		// Key 'B' is pressed, we will store as 11

	KBR1 = 1;		// scan keypress on 3rd row: 7, 8, 9, C
	KBR2 = 1;
	KBR3 = 0;
	KBR4 = 1;
	__delay_ms(5);
	if (KBC1 == 0)	return '7';		// Key '7' is pressed
	if (KBC2 == 0)  return '8';		// Key '8' is pressed
	if (KBC3 == 0)  return '9';		// Key '9' is pressed
    if (SENSPUERTA == 0)  return 'A';    
    //(if (KBC4 == 0) return 'C';		// Key 'C' is pressed, we will store as 12

	KBR1 = 1;		// scan keypress on 4th row: *, 0, #, D
	KBR2 = 1;
	KBR3 = 1;
	KBR4 = 0;
	__delay_ms(5);
	if (KBC1 == 0)	return '*';		// Key '*' is pressed, we will store as 14
	if (KBC2 == 0)  return '0';		// Key '0' is pressed
	if (KBC3 == 0)  return '#';		// Key '#' is pressed, we will store as 15
    if (SENSPUERTA == 0)  return 'A';    
	//if (KBC4 == 0) return 'D';		// Key 'D' is pressed, we will store as 13

	return 0xFF;					// if no key press, the register is 0xFF    
}

unsigned char KBDwaitDoor(void)
{
	// The pressed key.
	unsigned char KBDpressed = 0xFF;	
	
	// Wait until the key is pressed.
	do {
		KBDpressed = KBDrd();
        if(KBDpressed == 'A')
        {
            return KBDpressed;
        }
        ClrWdt();
	}while (KBDpressed == 0xFF);
	
	// Wait until the key is released.
	while (KBDrd() != 0xFF){
        ClrWdt();
    }
	
	return KBDpressed;
}

unsigned char KBDwait100ms(void)
{
	// The pressed key.
	unsigned char KBDpressed = 0xFF;	
    unsigned char ms = 0; 
	// Wait until the key is pressed.
    
	do {
		KBDpressed = KBDrd();
        ClrWdt();
        ms++;
        __delay_ms(1);
        if(ms == 100)
        { goto noKeyRelease; }
	}while (KBDpressed == 0xFF);
	
	// Wait until the key is released.
	while (KBDrd() != 0xFF){
        ClrWdt();
    }
noKeyRelease: 	
	return KBDpressed;
}

unsigned char KBDwait(void)
{
	// The pressed key.
	unsigned char KBDpressed = 0xFF;	
	
	// Wait until the key is pressed.
	do {
		KBDpressed = KBDrd();
        ClrWdt();
	}while (KBDpressed == 0xFF);
	
	// Wait until the key is released.
	while (KBDrd() != 0xFF){
        ClrWdt();
    }
	
	return KBDpressed;
}


