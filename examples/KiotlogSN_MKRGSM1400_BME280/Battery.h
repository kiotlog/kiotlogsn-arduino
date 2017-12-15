#define ADC_RESOLUTION 12
const float adc2mv = (1 / (float)((1 << ADC_RESOLUTION) - 1)) * (1 / 1.1316) * ( 1.0 / (330.0 / (1200.0 + 330.0)));

float readBattery()
{
    float battery_volts = analogRead(ADC_BATTERY);
    return battery_volts * adc2mv;
}