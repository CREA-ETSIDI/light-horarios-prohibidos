//Horarios prohibidos by Marce by CREA by tuviejosabroso
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <LiquidCrystal.h>
#include <TimeAlarms.h>

#define COLS 16
#define ROWS 2
#define NUM_ALARMAS 3

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
  /**PROGRAMAR HORARIOS PROHIBIDOS**/
  /**Recuerda ponerlos desfasados -4 min
   * Estructura de la matriz de horarios {diadelasemana, horas, minutos, DURACIÓN}
   * diadelasemana 1~7 y COMIENZA EN DOMINGO
  **/
  int Horarios_prohibidos_init[NUM_ALARMAS][4] = {
    {3,17,24,2},
    {5,11,26,0},
    {5,15,26,0}
  };
  /**PROGRAMAR APERTURA Y CIERRE**/
  /**Recuerda ponerlos desfasados -4 min
   * Estructura de la matriz de horarios {diadelasemana, horas, minutos, DURACIÓN}
   * diadelasemana 1~7 y COMIENZA EN DOMINGO
  **/
  int aperturaycierre[2][2] = {
    {8,26},
    {21,26}
  };
  
  Alarm.alarmRepeat(aperturaycierre[0][0], aperturaycierre[0][1], 0, EventoHDB);//ID alarma = 0
  Alarm.alarmRepeat(aperturaycierre[1][0], aperturaycierre[1][1], 0, EventoHP);//ID alarma = 1
  Alarm.alarmRepeat(Horarios_prohibidos_init[0][0], Horarios_prohibidos_init[0][1], Horarios_prohibidos_init[0][2], Horarios_prohibidos_init[0][3],EventoHP);//ID alarma = 2
  Alarm.alarmRepeat(Horarios_prohibidos_init[1][0], Horarios_prohibidos_init[1][1], Horarios_prohibidos_init[1][2], Horarios_prohibidos_init[1][3],EventoHP);//ID alarma = 3
  Alarm.alarmRepeat(Horarios_prohibidos_init[2][0], Horarios_prohibidos_init[2][1], Horarios_prohibidos_init[2][2], Horarios_prohibidos_init[2][3],EventoHP);//ID alarma = 4
  
  //check for initial status
  RTC.read(datetime);
  int flag = 0;
  for(int i = 0; i <NUM_ALARMAS; i++){
    if (weekday() == Horarios_prohibidos_init[i][0] && datetime.Hour >= Horarios_prohibidos_init[i][1]){
      if (datetime.Hour <= Horarios_prohibidos_init[i][1]+Horarios_prohibidos_init[i][3]){
        if (datetime.Minute <= Horarios_prohibidos_init[i][2]){flag = 1;}
      }
    }
  };
  if (flag == 1) {EventoHP();}
  else {EventoHDB();};  
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
    //Arreglamos el delay de 4 min que tiene.
    //Sino, se descomenta la siguiente línea y se borran los bucles:
    //print2digits(datetime.Minute);
    if (datetime.Minute <=55){
      print2digits(datetime.Minute+4);
    }
    else{
      print2digits(datetime.Minute-56);
    };
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
  delay(5000);
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
  delay(5000);
}
/**
   Funcion callback de fiesta yuju
*/
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
