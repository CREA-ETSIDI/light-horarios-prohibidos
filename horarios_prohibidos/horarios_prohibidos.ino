//Horarios prohibidos by Marce by CREA by tuviejosabroso
#include <LiquidCrystal.h>
#include <Time.h>
#include <DS1307RTC.h>

#define COLS 16
#define ROWS 2

#define tmDow2secs(t) (t.hour * 3600UL + t.mins * 60UL + t.secs)
typedef struct {
  uint8_t hour;
  uint8_t mins;
  uint8_t secs;
} dayOffset_t;

static LiquidCrystal lcd(12,11,5,4,3,2);
static tmElements_t tm;
static dayOffset_t td;
static bool malMomento, prevMomento;

void setup() {
  pinMode(8, OUTPUT);
  lcd.begin(COLS, ROWS);
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    lcd.print("Fallo de RTC");

  horasLibertad(); // Un primer print en pantalla que dice que todo está bien. Si no, se actualiza al otro estado durante la ejecución
}

void loop() {
  // Obtener la fecha y hora desde el chip RTC
  if (RTC.read(tm)) {
    imprimir_fecha_hora();

    td.hour = tm.Hour;
    td.mins = tm.Minute;
    td.secs = tm.Second;

    prevMomento = malMomento; // Almacenamos el estado anterior
    malMomento = instanteEnHorarioSilencio(td, tm.Wday); // Comprobamos en qué horario estamos

    if (prevMomento != malMomento) { // Solo actualizamos si ha cambiado el estado
      (malMomento) ? horasSilencio() : horasLibertad();
    }

    delay(450); // Tiempo de espera lo suficientemente alto como para no procesar estados en los que no hay cambio
  }
  else {
    lcd.clear();
    if (RTC.chipPresent()) {
      // El reloj esta detenido, es necesario ponerlo a tiempo
      lcd.setCursor(0, 0);
      lcd.print("DS1307 DETENIDO");
      lcd.setCursor(0, 1);
      lcd.print("EJECUTE PROGRAMA");
    }
    else {
      // No se puede comunicar con el RTC en el bus I2C, revisar las conexiones.
      lcd.print("  DS1307 NO SE");
      lcd.setCursor(0, 1);
      lcd.print(" PUDO DETECTAR");
    }
    delay(100); // Si limpias la pantalla muy seguido se ve rara
  }
}

/**
   Comprueba si el instante de tiempo pasado como argumento se encuentra en alguno de los horarios que debe haber silencio
   Cambia esta funcion para contemplar otros horarios
*/
uint8_t instanteEnHorarioSilencio(dayOffset_t t, uint8_t wDay) {
  static bool malMomento;
  malMomento = false;
  switch (wDay) {
    case dowTuesday:
      malMomento |= isBetween_timeIgnoreDOW(
        {17, 25, 0 }, 
        {19, 35, 0 },
        t);
      break;
    case dowThursday:
      malMomento |= isBetween_timeIgnoreDOW(
        {11, 25, 0 }, 
        {13, 35, 0 },
        t);
      break;
    case dowInvalid:
    default:
      break;
  }

  malMomento |= isBetween_timeIgnoreDOW(
    {21, 30, 0 }, 
    {8 , 30, 0 },
    t);
  return malMomento;
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
  lcd.setCursor(10, 0);
  lcd.print("SHHH!");
  digitalWrite(8, HIGH);
}
 
/**
   Funcion callback que desactiva el relevador en el pin 3 (horario de bien)
*/
void horasLibertad()
{
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

/**
   Comprueba si un dayOffset_t está entre un intervalo de horas time1 y time2
*/
bool isBetween_timeIgnoreDOW(dayOffset_t time1, dayOffset_t time2, dayOffset_t cmpre) {
  long t1Secs = tmDow2secs(time1);
  long t2Secs = tmDow2secs(time2);
  long cmpScs = tmDow2secs(cmpre);
  if (t1Secs < t2Secs) { // t1Secs is lower end of the interval
    return (t1Secs <= cmpScs && cmpScs <= t2Secs);
  }
  // else, t1Secs is upper end, so check belonging to the interval [t2Secs, t1Secs]
  return (cmpScs <= t2Secs || t1Secs <= cmpScs);
}
