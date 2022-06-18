// All variables
float corrienteNominal;
float voltajeNominal;
float mulCorrienteNominalTmpoInv;
float mulCorrienteNominalTmpoInst;
int tmpoSobreCorrInst;
float arranqueBajoVoltajeTmpoCte;
float arranqueBajoVoltajeTmpoInst;
float arranqueSobreVoltajeTmpoCte;
float arranqueSobreVoltajeTmpoInst;
int tmpoInstVoltaje;
int tmpoCteVoltaje;
float mulCorrienteArranqueRotorTranc;
float tmpoRotTrancado;
int numeroDeArranques;
int tmpoDeNumeroArranques;
float subMultTrabVacio;
int tmpoTrabVacio;
float factorDeServicio;
// StartPinVariables
int pinDeVoltaje = A0;
int pinDeCorriente = A8;
int pinSalidaDeMotor = 10;

//Setup de variables valor defecto
void SetupVariables(){
  tmpoInstVoltaje = 1000;
}

// Functions here
float leerVoltaje(){
  return analogRead(pinDeVoltaje) * 5.0 / 1023.0;
}

float leerCorrienteNominal(){
  float current = analogRead(pinDeCorriente) * 30 / 1023.0;
  return current / corrienteNominal;
}

boolean voltajeNoEsCorrecto(float voltaje){
  return voltaje <= 3.6 || voltaje >= 4.4;
}

void proteccionDeVoltaje(){
  float voltaje = leerVoltaje();
  if (voltajeNoEsCorrecto){
    int tiempoInicial = millis();
    while (millis() - tiempoInicial <= tmpoInstVoltaje){
      millis();
    }
    if (voltajeNoEsCorrecto){
      digitalWrite(pinSalidaDeMotor, LOW);
    }
  }
}