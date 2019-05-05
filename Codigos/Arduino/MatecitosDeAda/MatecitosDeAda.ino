int accion;
// constantes
#define SIN_ACCION     0;
#define ESPERAR_MATE   1;
#define PONER_YERBA    2;
#define PONER_AZUCAR   3;
#define SERVIR_AGUA    4;

//pines
#define alimentacionBT      23;
#define btPairing           49;
#define depositadorAzucar 	44;
#define depositadorYerba    45;
#define botonCantidades     46;

// objetos
SoftwareSerial bt(20, 21); // RX, TX
HX711 balYerba	(62, 63);
HX711 balAzucar	(64, 65);
HX711 balAgua	(66, 67);

//variables
int azucarDiponible;
int yerbaDisponible;
int aguaDisponible;
int cantAzucarPorMate;
int cantAzucarEnMate;
int cantYerbaPorMate;
int cantYerbaEnMate;
int cantAguaPorMate;
int cantAguaEnMate;
int sensorProximidad;

int correccionAzucar;
int correccionYerba;
int correccionAgua;

float calibration_factor = 2230;

void setup() {
  accion = SIN_ACCION;
  bt.begin(9600);
  bt.print("AT + NAMELosMatesDeAda");
  bt.print("AT + PINpass");

  //entradas
  pinMode(btPairing			  	, INPUT);
  pinMode(botonCantidades		, INPUT);
  //salidas
  pinMode(alimentacionBT      	, OUTPUT);
  pinMode(depositadorYerba	  	, OUTPUT);
  pinMode(depositadorAzucar	  	, OUTPUT);

  // balanzas
  balAzucar.set_scale();
  balAzucar.tare();
  balAgua.set_scale();
  balAgua.tare();
  balYerba.set_scale();
  balYerba.tare();
  //correcciones Balanzas queda para leer de SD
  correccionAgua = 0;
  correccionYerba = 0;
  correccionAzucar = 0;
}

void loop() {
  digitalWrite(alimentacionBT, !digitalRead(btPairing));

  switch (accion) {
    case SIN_ACCION:
      leerBT();
      break;

    case ESPERAR_MATE:
      if (sensorProximidad)
        accion = PONER_YERBA;
      break;

    case PONER_YERBA:
      if (primerMate || mateLavado) {
        if (cantYerbaPorMate > cantYerbaEnMate || !cantYerbaPorMate) {
          analogWrite(depositadorYerba, porcentaje(50)); //0-255
          if (!cantYerbaPorMate && ((bt.available() > 0 && bt.read()) || digitalRead(botonCantidades)))
            cantYerbaPorMate = cantYerbaEnMate;
        }
      } else
        accion = PONER_AZUCAR;
      break;

    case PONER_AZUCAR:
      if (cantAzucarPorMate > cantAzucarEnMate || !cantAzucarPorMate) {
        analogWrite(depositadorAzucar, porcentaje(50)); //0-255
        if (!cantAzucarPorMate && ((bt.available() > 0 && bt.read()) || digitalRead(botonCantidades)))
          cantAzucarPorMate = cantAzucarEnMate;
      }
      else
        accion = SERVIR_AGUA;
      break;

    case SERVIR_AGUA:
      if (cantAguaPorMate > cantAguaEnMate || !cantAguaPorMate) {
        if (!cantAguaPorMate && ((bt.available() > 0 && bt.read()) || digitalRead(botonCantidades)))
          cantAguaPorMate = cantAguaEnMate;
        analogWrite(depositadorAgua, porcentaje(50)); //0-255
      }
      else
        accion = ESPERAR_MATE;
      break;
  }
  seteoVaribles();
}


int porcentaje(int percent) {
  return (255 * percent) / 100;
}

void seteoVaribles() {
  int azucarDiponibleTemp = azucarDiponible;
  int yerbaDiponibleTemp = yerbaDiponible;
  int aguaDiponibleTemp = aguaDiponible;
  
  azucarDiponible = leerBalanza(balAzucar) + correccionAzucar;
  yerbaDiponible = leerBalanza(balYerba) + correccionYerba;
  aguaDiponible = leerBalanza(balAgua) + correccionAgua;

  if (azucarDiponibleTemp)
    cantAzucarEnMate = azucarDiponibleTemp - azucarDiponible;

  if (yerbaDiponibleTemp)
    cantYerbaEnMate = yerbaDiponibleTemp - yerbaDisponible;

  if (aguaDiponibleTemp)
    cantAguaEnMate = aguaDiponibleTemp - aguaDiponible;
}


float leerBalanza(HX711 bal) {
  float peso = bal.get_units(), 10;
  if (peso < 0)
    peso = 0.00;

  return peso;
}

void enviarInfo() {
  bt.print(1)
  bt.print(accion);

  if (accion == SACAR_CONTENIDO) {
    bt.print(ledSirviendo);
    bt.print((milis() % 500 > 250));
  }

  bt.print(ledDisponible);
  bt.print(accion == SIN_ACCION);
}

void leerBT() {
  if (bt.available() > 0)
    accion = bt.read();
}
