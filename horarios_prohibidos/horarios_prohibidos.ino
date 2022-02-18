//Horarios prohibidos by Marce by CREA by tuviejosabroso
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <LiquidCrystal.h>
#include <TimeAlarms.h>

#define COLS 16
#define ROWS 2

LiquidCrystal lcd(12,11,5,4,3,2);
tmElements_t datetime;
char DiaSemana[][4] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };

void setup() {
  pinMode(8, OUTPUT); 
  Serial.begin(9600);
  lcd.begin(COLS,ROWS);
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    lcd.print("Fallo de RTC");
  else
    lcd.print("Sincronizado con RTC");
  EventoHDB;
  //Programar horarios prohibidos
  //Recuerda ponerlos desfasados -4 min
  Alarm.alarmRepeat(dowTuesday, 17, 26, 0,EventoHP);
  Alarm.alarmRepeat(dowTuesday, 19, 26, 0,EventoHDB);
  Alarm.alarmRepeat(dowThursday, 11, 26, 0,EventoHP);
  Alarm.alarmRepeat(dowThursday, 13, 26, 0,EventoHDB);
  Alarm.alarmRepeat(21, 26, 0, EventoHP);
  Alarm.alarmRepeat(8, 26, 0, EventoHDB);
}

void loop() {
  imprimir_fecha();
}
/**
   Funcion auxiliar para imprimir siempre 2 digitos
*/
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}
void imprimir_fecha(){
  // Declaramos una estructura "tm" llamada datetime para almacenar
  // La fecha y la hora actual obtenida desde el chip RTC.
  // Obtener la fecha y hora desde el chip RTC
  if (RTC.read(datetime)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    print2digits(datetime.Hour);
    lcd.write(':');
    print2digits(datetime.Minute+4);
    lcd.write(':');
    print2digits(datetime.Second);
    lcd.setCursor(0, 1);
    lcd.print("Date: ");
    lcd.print(datetime.Day);
    lcd.write('/');
    lcd.print(datetime.Month);
    lcd.write('/');
    lcd.print(tmYearToCalendar(datetime.Year));
  }
  else {
    if (RTC.chipPresent()) {
      // El reloj esta detenido, es necesario ponerlo a tiempo
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("EL DS1307 ESTA DETENIDO");
      lcd.setCursor(0, 1);
      lcd.print("CORRE EL PROGRAMA PARA PONERLO A TIEMPO");
    }
    else {
      // No se puede comunicar con el RTC en el bus I2C, revisar las conexiones.
      lcd.clear();
      lcd.print("NO SE DETECTO EL DS1307");
      lcd.print("REVISA TUS CONEXIONES E INTENTA DE NUEVO");
    }
  };
  Alarm.delay(1000);
}
/**
   Funcion callback que activa el relevador en el pin 3 (horario prohibido)
*/
void EventoHP()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Horas del mal");
  digitalWrite(8, HIGH);
}
 
/**
   Funcion callback que desactiva el relevador en el pin 3 (horario de bien)
*/
void EventoHDB()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Horas de bien");
  digitalWrite(8, LOW);
}
void fiesta(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Partytime!");
  digitalWrite(8, LOW);
  for(int i=0;i<30; i++){
    digitalWrite(8, LOW);
    delay(100);
    digitalWrite(8, HIGH);
    delay(100);
  };
}
