/**
        Device            :  PIC18F46K22
*/
#include "mcc_generated_files/mcc.h"
#include "mcc_generated_files/memory.h"
#include "LCD.h"
#include "KEYPAD.h"

#define cleanBuff  for(i = 0; i <= 7; i++){ keyboard[i] = '\0';} //llena de ceros/null el buffer
#define keyNotAllowed        LCDclr(); LCDputs("     CLAVE      ");  LCDsc(2,1);  LCDputs("   INCORRECTA   "); __delay_ms(800)
#define deptoNotAllowed      LCDclr(); LCDputs("  DEPARTAMENTO  ");  LCDsc(2,1);  LCDputs("  NO PERMITIDO  "); __delay_ms(800)
#define PUERTA  LATDbits.LATD1 //ANTES D5 -CAMBIO POR SENOUt
#define EN  LATCbits.LATC5 //ENABLE MAX485 RECEIVER MODE

unsigned char i = 0, cursor, event, eventType;     
unsigned char keyboard[6], r[2];
unsigned char sentence[10]; 
unsigned char keyPress = 0; 
const    int  keyModeProg = 1234;
unsigned long result; 
unsigned long cpto = 0;
unsigned long disp = 0;
unsigned long read = 0; 
unsigned long apdepto = 0; 
unsigned short apEvent;     
unsigned long appto = 0;
unsigned long apclave = 0;
unsigned char buff[6]={0,0,0,0,0,0};

void recibirUART(void); 
void getInfo(void); 
void modeTest(void);
void modeProg(void);
void verModeProg(void);
void progDepto(void);
void borrarMem(void);
void editDepto(void);
void llamarDepto(void);
unsigned long ingresar(void);
unsigned long EEslongrd(unsigned long addr);
void EEslongwr(unsigned long addr, unsigned long data);
void plusPto(void);
unsigned long checkMem(void);
void eqcpto(unsigned long rs);
void progCacc(void);
void keyType(unsigned long apd);
void UARTgets1(void);

int main(void)
{

    SYSTEM_Initialize();
    EUSART1_Initialize();    
    PUERTA = 0;  
    LCDin();
    LCDclr();
    
    while(1)
    {   
        //recibirUART(); 
        ClrWdt();
        getInfo();
        //pinCall = 0;        
    }      
}

void getInfo(void)
{   
    __delay_ms(200);
    ClrWdt(); // C access to assembler insructions
    LCDclr(); // Limpia LCD
    LCDputs("INGRESE DEPTO.");

    LCDsc(2,1); // Fija cordenadas en LCD
    LCDputs("    N:"); LCDcon(); //Cursor OFF
    cleanBuff; //libera espacio
    i = 1; cursor = 7; 
    keyPress = KBDwaitDoor(); //espera a que se presione un boton en el teclado
  
    //--------------------------------------------------------------------------checar, porque debe llegar por RX
    /*   
    if(keyPress == 'A')
    {
        LCDclr();
        __delay_ms(3000);
        LCDputs(" PUERTA ABIERTA");
        PUERTA = 1; //pasa de estar cerrada 0 a abierta 1
        __delay_ms(3000);
        __delay_ms(3000);//*
        PUERTA = 0;
        while (PUERTA != 0) //monitorea hasta que se cierre la puerta
        {  PUERTA = 0; }   
        asm("goto _getInfo");  //regresa a getInfo              
    } 
    */

    if(keyPress == '#'){ LCDputc('*'); cursor++; verModeProg();} //manda a modo de prog
    //if(keyPress == '*'){ asm("goto _screen"); } //si se apreta * va a pantalla
    if(keyPress == '*'){ asm("goto _getInfo"); } //si se apreta * va a pantalla   
    else{
        cursor++;
        keyboard[0] = keyPress;
        LCDputc(keyboard[0]);
        do{
            ClrWdt();
f1:            
            keyboard[i] = KBDwaitDoor(); 
            if(keyboard[i] == '#')
            { keyboard[i] = '\0'; break; }

            if(keyboard[i] == '*')
            {                   
                asm("goto _getInfo");
            }
f2:         LCDputc(keyboard[i]);
            i++; cursor ++; 
        } while((i <= 7));
    }
    ClrWdt();
    result = atoi(keyboard);
        if (result == 0x0000)
        {   
            LCDclr();
            LCDputs("  DEPARTAMENTO  ");
            LCDsc(2,1); 
            LCDputs("  NO PERMITIDO  ");
            __delay_ms(800); 
        //    asm("goto _screen"); 
            asm("goto _getInfo"); 
        }   
         ClrWdt();
        DATAEE_WriteByte(5,0); //Para comprobar EEPROM     
        cpto = checkMem();
        apdepto = 10 + (12 * cpto); 
        // Buscar departamento y despues llamar al puerto asignado   
        
        unsigned long rd = 0; LCDcoff();
        
        for(unsigned long i = 0; i <= cpto; i++)
        {
            rd = EEslongrd(10 + (12 * i)); // 10 + (12 * i) -> Recorre direcciones deptos.
                if (rd == result)
                {   // Aqui se llamará al puerto por UART para intercomunicacion. 
                    LCDsc(1,1);
                        LCDputs(" LLAMANDO DEPTO. ");
                        read = EEslongrd(14 + (12 * i));
                        
                        //LLAMANDO... POR UART
                        UARTputc(65);
                        __delay_us(150);
                        UARTputc(10);//salto de linea
                        __delay_us(150);
                        UARTputc(13);//retorno de carro
                        __delay_us(150);
                        __delay_ms(3000);
                        
                        read = EEslongrd(14 + (12 * i));
                        printf("    N:%d",rd);
//                    deMux(read - 1); //ELIMINACION DE CODIGO 260220
                        CLRWDT();
                        Reset();
                //    asm("goto _screen");  //
                      asm("goto _getInfo");  
                }              
       } 
            //Cuando el departamento no existe 
            ClrWdt();
            LCDclr(); 
            LCDputs("DEPTO. NO EXISTE"); 
            __delay_ms(800);
            //asm("goto _screen");     
            asm("goto _getInfo");

}

void controlAcceso(int auxKey)
{
    unsigned long cpto;
//     Modo CAcceso 
    if (auxKey == 0)
    {  
        deptoNotAllowed;
        asm("goto _getInfo"); 
    } 

    DATAEE_WriteByte(5,0); // Checar memoria        
    cpto = checkMem();
    // Buscar departamento y despues llamar al puerto asignado        
    unsigned long rd = 0, iaux = 0; 
    for(unsigned long i = 1; i <= cpto; i++)
    {    
        rd = EEslongrd(10 + (12 * i)); // 10+(12 * i) -> Recorre direcciones deptos.
            if (rd == auxKey)
            {
                iaux = i;
                keyType(iaux);
                CLRWDT();
            }   
    } 
    LCDclr();
   LCDputs("DEPTO. NO EXISTE");
    CLRWDT();

    __delay_ms(800);
    asm("goto _getInfo"); 
     
    return;
}

void keyType(unsigned long apd)
{
    unsigned long auxKey = 0;
    
    LCDclr(); 
    LCDputs("CLAVE");                 
    LCDsc(2,6);
    LCDputs("N:");

    cleanBuff;

    unsigned int i = 8, c = 0;        
    while ( (i>=8) && (i<=14) )
    {         
        keyPress = KBDwait();
        keyboard[c] = keyPress; 
        if (keyboard[0] == '*')
            {   asm("goto _getInfo");}
        if (keyPress == '*') 
            {              
                i--; c--; LCDsc(2,i); putch(' '); LCDsc(2,i); 
                keyboard[c] = '\0';      
            }        
        else {
                if (keyPress == '#')
                {  break; }                               
                else 
                {
                    putch('*');
                    i++; c++;
                }
            }
    }  

    auxKey = atoi(keyboard);

    if (auxKey == 0)
    {
        keyNotAllowed;
        asm("goto _getInfo");
    }   
    
    unsigned long rd = 0; 
    rd = EEslongrd(18 + (12 * apd));
    if (rd == auxKey)
    {
        LCDclr();
        LCDputs(" PUERTA ABIERTA ");
        PUERTA = 1; 
        __delay_ms(3000);

        while (PUERTA != 0)
        {  PUERTA = 0; }

        asm("goto _getInfo");
    }  

    else
    {
        keyNotAllowed;
        asm("goto _getInfo");
    }
}

void verModeProg(void)
{   
    ClrWdt();
    cleanBuff;
    int auxKey = 0; i = 0;  
    keyPress = KBDwait();
    if(keyPress == '*') { asm("goto _getInfo");  }
    if(keyPress == '#')
    {
        LCDputc('*');
        do{
            ClrWdt();
            keyboard[i] = KBDwait(); 
            if(keyboard[i] == '#') {keyboard[i] = '\0'; break;}
            if(keyboard[i] == '*') { asm("goto _getInfo"); } 
            LCDputc('*');
            i++; cursor++;
        } while((i <= 5));
        
        auxKey = atoi(keyboard);
        
        if(auxKey == keyModeProg)
        {
            modeProg();
        }   
        
        else{
            asm("goto _getInfo");
        }
    
    }
    
    if((keyPress != '*') || (keyPress != '#'))
    {
        keyboard[i] = keyPress; 
        i++;
        LCDputc('*');      
        do{
            ClrWdt();
            keyboard[i] = KBDwait(); 
            if(keyboard[i] == '#') {keyboard[i] = '\0'; break;}
            if(keyboard[i] == '*') { asm("goto _getInfo"); } 
            LCDputc('*');
            i++; cursor++;
        } while((i <= 5));
        
        auxKey = atoi(keyboard);
        // Llamar a control de acceso
        controlAcceso(auxKey);        
    }    

    //asm("goto _screen");
    asm("goto _getInfo");
    
    return;
}

void modeProg(void)
{   unsigned char a = 1;   
    ClrWdt();
    LCDclr();  
    LCDputs("PROGRAMAR DEPTO."); //Opcion Inicial 
    
menusel:
    LCDsc(2,1);
    LCDputs("<4  Aceptar#  6>");
        //flag.teclado = 0; 
       keyPress = KBDwait(); 
        
switch(keyPress)
    {
    case '4': 
        a--;
        if(a == 0)
        { a = 6 ; } 
        goto selmensaje;
    case '6': 
        a++;
        if(a == 7)
        { a = 1; }
        goto selmensaje;   
    }
    
selmensaje: 
    LCDsc(1,1);

switch(a)
    {
    case 1: 
        LCDputs("PROGRAMAR DEPTO.");        
        if(keyPress == '#')
        {progDepto();}
        else
        {goto menusel;}
    case 2:
        LCDputs(" BORRAR MEMORIA ");        
        if(keyPress == '#')
        {borrarMem();}
        else
        {goto menusel;}        
    case 3:
        LCDputs("  REVISAR PROG.");
        if(keyPress == '#')
        {editDepto();}
        else
        {goto menusel;}
    case 4:
        LCDputs("  PROG. C.ACC  ");
        if(keyPress == '#')
        {progCacc();}
        else
        {goto menusel;}            
    case 5:
        LCDputs("   MODO PRUEBA   ");
        if(keyPress == '#')
        {modeTest();}
        else
        {goto menusel;}           
    case 6:
        LCDputs("     SALIR       ");
        if(keyPress == '#')
        //{asm("goto _screen");}
       {asm("goto _getInfo");} 
        else
        {goto menusel;}        
    default:       
        goto menusel; 
    }
}

void progCacc(void)
{
    unsigned long cpto = 0;
    LCDclr();
    LCDputs("MARQUE EL NUMERO"); 
    LCDsc(2,1); 
    LCDputs("  DEPTO:");  
    ingresar();
    // Primero se debe buscar departamento
    // Despues se debe apuntar a la ultima direccion programada y programar en la siguiente, el departamemto.           
        if (result == 0)
        {
            DATAEE_WriteByte(5,0); // Comprobar EEPROM              
            LCDclr();
            LCDputs("  DEPARTAMENTO  ");
            LCDsc(2,1); 
            LCDputs("  NO PERMITIDO  ");
            __delay_ms(800); 
            asm("goto _progCacc");
        }
    cpto = 0;                      
    cpto = checkMem();  
        unsigned long rd = 0, caux = 0; // caux -> Contador auxiliar direcciones
        for(unsigned int i = 0; i <= cpto; i++)
        {    
            rd = EEslongrd(10 + (12 * i)); // 10+(12 * i) -> Recorre direcciones deptos.
            if (rd == result)
            {
                //Se puede programar control de acceso.
                caux = i; // Se guarda la direccion a la que se acceso. 
                goto progCacceso; 
            }    
        } 
    //Cuando no existe el departamento.    
    LCDclr();
    LCDputs("DEPTO. NO EXISTE"); 
    __delay_ms(800);
    asm("goto _progCacc"); 
 
progCacceso:
    LCDclr();
    LCDputs("MARQUE LA CLAVE"); 
    LCDsc(2,1); 
    LCDputs("  CLAVE:");   
    ingresar();
    // Primero se debe buscar departamento
    // Despues se debe apuntar a la ultima direccion programada y programar en la siguiente, el departamemto.         
        if (result == 0)
        {
            DATAEE_WriteByte(5,0); // Comprobar EEPROM              
            LCDclr();
            LCDputs("     CLAVE     ");
            LCDsc(2,1); 
            LCDputs("  NO PERMITIDA  ");
            __delay_ms(800);
            goto progCacceso; 
        }

    apclave = 18 + (12 * caux);         
    EEslongwr(apclave, result); // En la siguiente localidad disponible
    rd = EEslongrd(apclave);
        if (result != rd)
           {
            LCDclr();
            LCDsc(1,1); 
            LCDputs("CODIGO DE ERROR:"); 
            LCDsc(2,6);
            LCDputs("002");
            __delay_ms(800);
            asm("goto _progCacc"); 
           } 
        asm("goto _progCacc"); 
        
    return;        
}

void modeTest(void)
{
 modeTestAgain: 
    LCDclr();  
    LCDputs("   PRESIONE #   "); //Opcion Inicial
    LCDsc(2,1);
    LCDputs("  PARA EMPEZAR  ");
    
    while(KBDwait() != '#');

p14:   
    LCDclr(); 
    LCDputs("   PUERTOS 1-4   ");    
 
    askA1:        
        keyPress = KBDwait();
        switch(keyPress)
        {
            case '4':
                goto p2940;
            case '6':
                goto p516;
            case '#':
                goto t14;
            case '*':
                asm("goto _modeProg");
            default: 
                goto askA1;
        }    

        t14:
        while(1)
        {   
            for(unsigned char u = 0; u < 4; u++)
            {
                for(unsigned char n = 0; n < 4; n++)
                {   
                    LCDsc(2,1); LCDcoff();
                    printf("   PUERTO: %d   ",n+1);                   
 //                   deMuxTest(n);// ELIMINACION DE CODIGO 260220
                }
            }
            LCDsc(2,1);
            LCDputs("    FIN  1-4    "); 
            break;
        }
    
p516:
    LCDclr(); 
    LCDputs("   PUERTOS 5-16   ");    
    askA2:        
        keyPress = KBDwait();
        switch(keyPress)
        {
            case '4':
                goto p14;
                break;
            case '6':
                goto p1728;
                break;
            case '#':
                goto t516;
                break;
            case '*':
                asm("goto _modeProg");
                break;
            default: 
                goto askA2;
                break;
        }   
        t516:
        while(1)
        {   
            for(unsigned char u = 0; u < 4; u++)
            {
                for(unsigned char u = 4; u < 16; u++)
                {           
                    LCDsc(2,1); LCDcoff();
                    printf("   PUERTO: %d   ",u+1);                   
 //                   deMuxTest(u);//ELIMINACION DE CODIGO 260220
                }
            }
            LCDsc(2,1);
            LCDputs("    FIN 5-16    "); 
            break;
        }
                   
p1728:    
    LCDclr(); 
    LCDputs(" PUERTOS 17-28   "); 
    askA3:        
        keyPress = KBDwait();
        switch(keyPress)
        {
            case '4':
                goto p516;
                break;
            case '6':
                goto p2940;
                break;
            case '#':
                goto p1728;
                break;
            case '*':
                asm("goto _modeProg");
                break;
            default: 
                goto askA3;
                break;
        }    
        while(1)
        {   
            for(unsigned char u = 0; u < 4; u++)
            {
                for(unsigned char u = 16; u < 28; u++)
                {  
                    LCDsc(2,1); LCDcoff();
                    printf("   PUERTO: %d   ",u+1);                   
//                    deMuxTest(u);//ELIMINACION DE CODIGO 260220
                }
            }
            LCDsc(2,1);
            LCDputs("    FIN 17-28    ");           
            break;             
        }
 
p2940:
    LCDclr(); 
    LCDputs("   PUERTOS 29-40   ");     
    askA4:        
        keyPress = KBDwait();
        switch(keyPress)
        {
            case '4':
                goto p1728;
                break;
            case '6':
                goto p14;
                break;
            case '#':
                goto p2940;
                break;
            case '*':
                asm("goto _modeProg");
                break;
            default: 
                goto askA4;
                break;
        }    
        while(1)
        {   
            for(unsigned char u = 0; u < 4; u++)
            {
                for(unsigned char u = 28; u < 40; u++)
                { 
                    LCDsc(2,1); LCDcoff();
                    printf("   PUERTO: %d   ",u+1);                   
//                    deMuxTest(u); //ELIMINACION DE CODIGO 260220
                }
            }
            LCDsc(2,1);
            LCDputs("    FIN 29-40    ");           
            break;              
        }
    
endTest:    
    LCDclr(); 
    LCDputs("  P. FINALIZADA");
    __delay_ms(1800);
    
    LCDclr();
    LCDputs(" EMP. DE NUEVO?");
    
    while(KBDwait() != '#') 
    { 
        keyPress = KBDwait();
        if(keyPress == '*')
        {
            asm("goto _modeProg");
        }
    }
    goto modeTestAgain;
         
    return; 
}

void progDepto(void)
{ 
a1: LCDclr();
        LCDputs("EMPEZAR EN PTO.");
    LCDsc(2,7);
        LCDputs("N:"); LCDcon();
        ingresar();
        
        if(result == 0)
        {   LCDclr();
                LCDputs("    PTO. NO    ");
            LCDsc(2,1);
                LCDputs("   PERMITIDO   ");
            LCDcoff(); 
            __delay_ms(800); 
            goto a1;
        }
        if (result < 40)
        {   cpto = result;   }
        else
        {   LCDclr();
                LCDputs("   MAX. PUERTO  ");
            LCDsc(2,1);
                LCDputs("    SUPERADO    ");
            __delay_ms(800);
            goto a1;  
        }
ppuerto:     
        LCDclr();
        printf("PUERTO N:%d", cpto);                 
pdepto: 
        LCDsc(2,1); 
        LCDputs("DEPTO.N:");
        ingresar();     
        if (result == 0)
        {   
            //plusPto();
            cpto++;
            if (cpto > 40)
            {   cpto--;
                LCDclr();
                LCDputs("   MAX. PUERTO  ");
                LCDsc(2,1);
                LCDputs("    SUPERADO    ");
                __delay_ms(800);
                goto a1; 
            }              
            if(cpto > checkMem())
            { eqcpto(cpto);}
            __delay_ms(300);
            goto ppuerto;
        } 
        
        DATAEE_WriteByte(5,0); //Para comprobar algun error en la EEPROM
        unsigned long rd = 0; 
        for(unsigned long i = 0; i <= cpto; i++)
        {    
            rd = EEslongrd(10 + (12 * i)); // 10+(12 * i) -> Recorre direcciones deptos.
            if (rd == result)
            {   LCDclr();
                   LCDputs("DEPTO. YA EXISTE");
                __delay_ms(800);
                goto ppuerto;
            }   
        } 
        
    apdepto = 10 + (12 * cpto); 
    appto = 14 + (12 * cpto);
            
    EEslongwr(apdepto,result);
    EEslongwr(appto,cpto);
    
    rd = 0;   
    rd = EEslongrd(apdepto);
        if (result != rd)
           {    LCDclr(); 
                    LCDputs("CODIGO DE ERROR:"); 
                LCDsc(2,6);
                    LCDputs("002");
                goto ppuerto; 
           }
    
        else
           {
                cpto++;
                if (cpto > 40)
                {   cpto--; 
                    LCDclr();
                        LCDputs(" OK - SIN PTOS. ");
                    LCDsc(2,1);
                        LCDputs("   DISPONIBLES  ");
                    __delay_ms(800);
                    goto a1; 
                }                
                if(cpto > checkMem())
                { eqcpto(cpto);}
                   
                CLRWDT();
                goto ppuerto;
           }
    
    return;
}

unsigned long ingresar(void)
{   
    repIn:
    result = 0; 
    for (unsigned int i = 0; i <= 6; i++) { keyboard[i] = '\0'; }  
    for (unsigned int i = 9; i<=16; i++) {  LCDsc(2,i); LCDputc(' '); LCDsc(2,i); }
    LCDsc(2,9);
    
        unsigned int i = 9, c = 0;  
        
        while ( (i >= 9) && (i <= 16) )
        {         
            keyPress = KBDwait();
            keyboard[c] = keyPress; 
            if (keyboard[0] == '*')
                {asm("goto _modeProg");}
            if (keyPress == '*') 
                {
                    goto repIn;   
                }        
            else {
                    if (keyPress == '#')
                    {  break; }                               
                    else 
                    {
                        LCDputc(keyboard[c]);
                        i++; c++;
                    }
                }
        }
        
        result = atoi(keyboard);
    return result;       
}

void borrarMem(void)
{   
    LCDclr();
    LCDputs("SALIR:* BORRAR:#");
    keyPress = KBDwait();
    if (keyPress == '#')
        {  
            LCDclr();
            LCDputs("  --BORRANDO--   ");        
            for (unsigned int i = 0; i <= 1023; i++)
            {
                DATAEE_WriteByte(i,'\0');
                ClrWdt();
            }        
            EEslongwr(0,1); /// Inicializar contador departamentos
            LCDclr(); 
            LCDcoff();
            LCDputs("  MEM. BORRADA  ");         
            __delay_ms(1000);
            asm("goto _modeProg");
        }
    else
        {
            asm("goto _modeProg");
        }  
    
    return;
}

void editDepto(void)
{   
    unsigned long aptoaux = 0; 
typepto:
    ClrWdt();
    LCDclr(); 
        LCDputs(" EMPEZAR EN PTO.");
    LCDsc(2,7);
        LCDputs("N:");
    LCDcon();
    ingresar();   
   
    if(result == 0)
    {   LCDclr();
        LCDputs("    PTO. NO    ");
        LCDsc(2,1);
        LCDputs("   PERMITIDO   ");
        LCDcoff(); 
        __delay_ms(800);
        goto typepto; 
    }

    if (result <= 40)
    {
    unsigned long rd = 0; 
        rd = EEslongrd(10 + (12 * result)); // 10+(12 * i) -> Recorre direcciones deptos.
        
        if(rd != '\0')
        { aptoaux = result;}
        
        else
        {   LCDclr();
            LCDputs("    PTO. NO    ");
            LCDsc(2,1);
            LCDputs("   PROGRAMADO  ");
            LCDcoff(); 
            __delay_ms(800);
            
            goto typepto;
        }
    }
    else 
    {   LCDclr();
        LCDputs("   MAX. PUERTO  ");
        LCDsc(2,1);
        LCDputs("    SUPERADO    ");
        __delay_ms(800);
        goto    typepto;  
    }
    
ppuerto:
    CLRWDT();
    LCDclr();
    printf("PUERTO N:%ld",aptoaux);    
    LCDcoff();
  
pdepto: 
    if (aptoaux > 40)
    {   
        LCDclr();
        LCDputs("   MAX. PUERTO  ");
        LCDsc(2,1);
        LCDputs("    SUPERADO    ");
        __delay_ms(800);
        goto    typepto; 
    } 

    DATAEE_WriteByte(5,0); //Para comprobar algun error en la EEPROM
    
        unsigned long rd = 0; 
        rd = EEslongrd(10 + (12 * aptoaux)); // 10 + (12 * i) -> Recorre direcciones deptos

        LCDsc(2,1); 
        printf("N.DEPTO:%ld",rd);
        
        keyPress = KBDwait();       

        cpto = 0;
        cpto = checkMem();
        switch(keyPress)
        {
            case '4': 
                aptoaux--;
                if(aptoaux == 0)
                { aptoaux = cpto; } 
                goto dispuerto;
            case '6': 
                aptoaux++;
                if(aptoaux == cpto)
                { aptoaux = 1; }
                goto dispuerto; 
            case '#':
            {
                CLRWDT();
                LCDclr();
                printf("PUERTO N:%ld",aptoaux); 
                
                LCDsc(2,1); 
                LCDputs("N.DEPTO:");
                LCDcon();
                ingresar();  
                // Primero se debe buscar departamento
                // Despues se debe apuntar a la ultima direccion programada y programar en la siguiente, el departamemto.    

                if (result == 0)
                {
                    goto wrdepto; 
                }
                cpto = 0; 
                cpto = checkMem();
                    DATAEE_WriteByte(5,0); //Para comprobar algun error en la EEPROM
                    rd = 0; 
                    for(unsigned long i = 0; i <= cpto; i++)
                    {    
                        rd = EEslongrd(10 + (12 * i)); // 10 + (12 * i) -> Recorre direcciones deptos.
                        if (rd == result)
                        {
                            LCDclr();
                            LCDputs("DEPTO. YA EXISTE");
                            __delay_ms(800);
                            goto ppuerto;
                        }   
                    }
                
                wrdepto:     
                    apdepto = 10 + (12 * aptoaux); 
                    appto = 14 + (12 * aptoaux);

                    EEslongwr(apdepto,result);
                    EEslongwr(appto,aptoaux);

                    LCDclr();
                    LCDputs("   EDITADO OK   ");
                    CLRWDT(); 
                    LCDcoff();
                    __delay_ms(800);
                    goto dispuerto; 
            }
            case '*':
                asm("goto _modeProg");
        }        
dispuerto:
    CLRWDT();
    LCDclr();
    printf("PUERTO N:%ld",aptoaux);    
    LCDcoff();      
    goto pdepto;
    
    return;
}

void EEslongwr(unsigned long addr, unsigned long data)
{  
   unsigned char i;
   for(i = 0; i < 4; ++i)
   {
     DATAEE_WriteByte(addr + i, *((unsigned char *)(&data) + i));
   }
   
   return;
}

unsigned long EEslongrd(unsigned long addr)
{  unsigned char i;
   unsigned long data;

   for(i = 0; i < 4; ++i)
   {
      *((unsigned char *)(&data) + i) = DATAEE_ReadByte(addr + i);
   }
   return data;
}

unsigned long checkMem(void)
{   unsigned long cont; //Contador puertos programados
    cont = EEslongrd(0x0000);
    return cont; 
}

void eqcpto(unsigned long rs)
{   EEslongwr(0,rs);
    return;
}

void plusPto(void)
{   unsigned long cont; 
    cont = EEslongrd(0x0000); 
    cont++; 
    EEslongwr(0x0000,cont);
    return;
}

void UARTgets1()
  {
    int i = 0;  // this is to point into the array at the position we are working at.
    do  // start of a loop
    {
      while(!RCIF) {};  // kill time until a character has arrived.  (note this blocks other operations while waiting!)
      sentence[i] = RCREG;  // copy the byte in RCREG to position i in the MyString array.
    
    }while(sentence[i++] != '\n'); // keep repeating, moving i to the next position until \n is found.
  }

void recibirUART(void)
{   
	unsigned char deptoTemp[2]={0,0,0};
	EN = 1;
	
		LCDclr(); // Limpia LCD
		
        LCDputs("RECIBIENDO...");
        LCDsc(2,1); 
		
        UARTgets1();
		
        //LCDputs(sentence);
        __delay_ms(500);
		deptoTemp[0]=sentence[4];
		deptoTemp[1]=sentence[5];
		deptoTemp[2]=sentence[6];
		
        if(sentence[7]=='A')
        {        
        LCDclr();
        LCDputs(" PUERTA ABIERTA");
        LCDsc(2,1);
        LCDputs(" DEPTO. NO.");
        LCDsc(2,13);
		LCDputs(deptoTemp);
		
        PUERTA = 1; //pasa de estar cerrada 0 a abierta 1
        __delay_ms(3000);
        __delay_ms(3000);//
        PUERTA = 0;
        memset(sentence,'0',10);
		memset(deptoTemp,'0',2);
        }

    return;
}

/**
 End of File
*/