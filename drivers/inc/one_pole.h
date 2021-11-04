/*
  ==============================================================================

    one_pole.h
    Author:  Jan Janssen
    Source:  https://www.earlevel.com/main/2012/12/15/a-one-pole-filter/

  ==============================================================================
*/


/*
************************************************************************************************************************
*
************************************************************************************************************************
*/

#ifndef ONE_POLE_h
#define ONE_POLE_h

/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdint.h>

/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

typedef enum {
    type_lowpass = 0,
    type_highpass
} one_pole_filter_types_t;

typedef struct ONE_POLE_FILTER_T {
    int type;
    double a0, b1;
    double Fc;
    double z1;
    uint32_t samplefrequency;
} one_pole_filter_t;


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

void one_pole_init(one_pole_filter_t *filter, uint32_t samplefreq, one_pole_filter_types_t filtertype);
void one_pole_set(one_pole_filter_t *filter, float Fc);
float one_pole_process(one_pole_filter_t *filter, float in);



#endif // ONE_POLE_h