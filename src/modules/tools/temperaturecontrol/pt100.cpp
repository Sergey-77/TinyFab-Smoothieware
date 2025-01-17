#include "pt100.h"
#include "libs/Kernel.h"
#include "libs/Pin.h"
#include "Config.h"
#include "checksumm.h"
#include "Adc.h"
#include "ConfigValue.h"
#include "StreamOutputPool.h"

#include <fastmath.h>

#define UNDEFINED -1

// Manual http://smoothieware.org/3d-printer-guide#toc12

#define pt100linear_checksum               CHECKSUM("pt100linear")        // if 1 then use slope/yintercept, if 0 use
                                                                          // a, b, c  in quadratic polynomial
#define thermistor_pin_checksum            CHECKSUM("thermistor_pin")
#define ampmod1_pin_checksum               CHECKSUM("ampmod1_pin")        // UP! printer uses this to "energize" the RTD
#define ampmod2_pin_checksum               CHECKSUM("ampmod2_pin")        // set as nc if you don't need to energize RTD
#define slope_checksum                     CHECKSUM("slope")              // We are looking RTD as "linear" so tune your
#define yintercept_checksum                CHECKSUM("yintercept")         // SLOPE and Y-Intercept to have minimal error
                                                                          // in the region you use 

#define pt100_a_checksum                    CHECKSUM("pt100_a")            // a value
#define pt100_b_checksum                    CHECKSUM("pt100_b")            // b value
#define pt100_c_checksum                    CHECKSUM("pt100_c")            // c value

PT100::PT100()
{
    this->bad_config = false;
    this->use_ampmod1= false;
    this->use_ampmod2= false;
    this->min_temp= 999;
    this->max_temp= 0;
    this->a = 0;             //use as slope or a 
    this->b = 0;             //use as yintercept or b
    this->c = 0;
    this->use_linear= true;  // true: use linear, false: polynomial 
}

PT100::~PT100()
{
}

void PT100::UpdateConfig(uint16_t module_checksum, uint16_t name_checksum)
{
    this->thermistor_pin.from_string(THEKERNEL->config->value(module_checksum, name_checksum, thermistor_pin_checksum )->required()->as_string());
    this->ampmod1_pin.from_string(   THEKERNEL->config->value(module_checksum, name_checksum, ampmod1_pin_checksum    )->by_default("nc")->as_string());
    this->ampmod2_pin.from_string(   THEKERNEL->config->value(module_checksum, name_checksum, ampmod2_pin_checksum    )->by_default("nc")->as_string());
    this->use_linear = THEKERNEL->config->value(module_checksum, name_checksum,  pt100linear_checksum)->by_default(1)->as_number() > 0;

    if (this->use_linear){
      this->a = THEKERNEL->config->value(module_checksum, name_checksum, slope_checksum      )->by_default(  0.0257604875F )->as_number(); // default value for UP! 
      this->b = THEKERNEL->config->value(module_checksum, name_checksum, yintercept_checksum )->by_default(-18.54F         )->as_number(); // PT100 in the hotend
    }else{
      this->a = THEKERNEL->config->value(module_checksum, name_checksum, pt100_a_checksum )->by_default( 0.000000174674754F )->as_number(); // default values
      this->b = THEKERNEL->config->value(module_checksum, name_checksum, pt100_b_checksum )->by_default( 0.022383F          )->as_number(); // for Cetus mk3
      this->c = THEKERNEL->config->value(module_checksum, name_checksum, pt100_c_checksum )->by_default( -4.00648F          )->as_number(); // 
    }
    THEKERNEL->adc->enable_pin(&thermistor_pin);

    if(this->ampmod1_pin.connected()){
        this->ampmod1_pin.as_output();
        this->ampmod1_pin.set(true);
        this->use_ampmod1= true;
    }

    if(this->ampmod2_pin.connected()){
        this->ampmod2_pin.as_output();
        this->ampmod2_pin.set(true);
        this->use_ampmod2= true;
    }

}

float PT100::get_temperature()
{
    if(bad_config) return infinityf();
    float t= adc_value_to_temperature(new_thermistor_reading());
    // keep track of min/max for M305
    if(t > max_temp) max_temp = t;
    if(t < min_temp) min_temp = t;
    return t;
}

void PT100::get_raw()
{
    if(this->bad_config) {
       THEKERNEL->streams->printf("WARNING: The config is bad for this temperature sensor\n");
    }
    int adc_value= new_thermistor_reading();
    float t = adc_value_to_temperature(adc_value);
    THEKERNEL->streams->printf("PT100: adc= %d, temp= %f\n", adc_value, t);
    min_temp= max_temp= t;
}

float PT100::adc_value_to_temperature(uint32_t adc_value)
{
    const uint32_t max_adc_value= THEKERNEL->adc->get_max_value();
    if ((adc_value >= max_adc_value) || (adc_value == 0))
        return infinityf();

    float t;

    if (this->use_linear){
        // We are using a very short range of temperatures for 3D printing
        // so looking at 150-350C span any RTD is almost linear hence we
        // use the simple linear equation allowing slope and y-intersect to 
        // be configurable so user can tune for particular RTD and tune for 
        // minimal error on the required span
        t = adc_value * this->a + this->b;
    } else {
    	// use quadratic polynomial to calculate temperature from ADC value
    	 t = this->a * adc_value * adc_value + this->b * adc_value + this->c;
    }
    return t;
}

int PT100::new_thermistor_reading()
{
    int t;

    if(this->use_ampmod1) this->ampmod1_pin.set(true);
    if(this->use_ampmod2) this->ampmod2_pin.set(true);

    // filtering now done in ADC
    t = THEKERNEL->adc->read(&thermistor_pin);

    if(this->use_ampmod1) this->ampmod1_pin.set(false);
    if(this->use_ampmod2) this->ampmod2_pin.set(false);

    return t;
}
