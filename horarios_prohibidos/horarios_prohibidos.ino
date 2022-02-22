//Horarios prohibidos by Marce by CREA by tuviejosabroso
#include <LiquidCrystal.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>

#define COLS 16
#define ROWS 2

static LiquidCrystal lcd(12,11,5,4,3,2);
static tmElements_t tm;

void setup() {
  pinMode(8, OUTPUT);
  lcd.begin(COLS, ROWS);
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    lcd.print("Fallo de RTC");
  else
    lcd.print("Sincronizado con RTC");
  
  // Habría que hacer un check para tener claro que no estamos en horarios malos
  // Pero el error es despreciable, ya que implica que haya alguien que lo sepa en el momento
  // Y solo pasa una vez, o cuando se reconecta la alimentación
  horasLibertad();
  // Igualmente me portaré bien y pondré un aviso
  lcd.setCursor(10, 0);
  lcd.print("Undef");
  
  // Programar horarios prohibidos
  //Alarmas semanales
  Alarm.alarmRepeat(dowTuesday , 17, 25, 0, horasSilencio);
  Alarm.alarmRepeat(dowTuesday , 19, 35, 0, horasLibertad);
  Alarm.alarmRepeat(dowThursday, 11, 25, 0, horasSilencio);
  Alarm.alarmRepeat(dowThursday, 13, 35, 0, horasLibertad);
  //Alarmas diarias (nocturna/diurna, ¿es necesario? I'll never know)
  Alarm.alarmRepeat(21, 30, 0, horasSilencio);
  Alarm.alarmRepeat( 8, 30, 0, horasLibertad);
}

void loop() {
  // Obtener la fecha y hora desde el chip RTC
  if (RTC.read(tm)) {
    imprimir_fecha_hora();
    delay(50);
  }
  else {
    if (RTC.chipPresent()) {
      // El reloj esta detenido, es necesario ponerlo a tiempo
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DS1307 DETENIDO");
      lcd.setCursor(0, 1);
      lcd.print("EJECUTE PROGRAMA");
      delay(100);
    }
    else {
      // No se puede comunicar con el RTC en el bus I2C, revisar las conexiones.
      lcd.clear();
      lcd.print("  DS1307 NO SE");
      lcd.setCursor(0, 1);
      lcd.print(" PUDO DETECTAR");
      delay(100);
    }
  }
}

/**
   Imprime la fecha desde la variable global "tm"
*/
void imprimir_fecha_hora(){
  // Hora en la fila superior
  lcd.setCursor(0, 0);
  print2digits(tm.Hour);
  lcd.write(':');
  print2digits(tm.Minute);
  lcd.write(':');
  print2digits(tm.Second);
  // Fecha en la fila inferior
  lcd.setCursor(0, 1);
  print2digits(tm.Day);
  lcd.write('/');
  print2digits(tm.Month);
  lcd.write('/');
  lcd.print(tmYearToCalendar(tm.Year));
}

/**
   Funcion callback que activa el relevador en el pin 3 (horario prohibido)
*/
void horasSilencio()
{
  lcd.clear();
  lcd.setCursor(10, 0);
  lcd.print("SHHH!");
  digitalWrite(8, HIGH);
}
 
/**
   Funcion callback que desactiva el relevador en el pin 3 (horario de bien)
*/
void horasLibertad()
{
  lcd.clear();
  lcd.setCursor(10, 0);
  lcd.print("Yay!!");
  digitalWrite(8, LOW);
}

/**
   Funcion auxiliar para imprimir siempre 2 digitos
*/
void print2digits(unsigned char number) {
  if (number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}
