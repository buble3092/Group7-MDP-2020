
#include <SharpIR.h>


/*  In order to get the curve fit equation of a sensor,
	take the mean raw voltage of the sensor (sample size = 50)
	when obstacle is x cm away (5 <= x <= 25, x++).
	Plot distance away as x, and raw voltage as y on an online website.
	Obtain curve fit function, put it in switch statement.

	Repeat above steps for however many sensors used.
*/

SharpIR::SharpIR(uint8_t _sensorType, uint8_t _sensorPin)
{
	sensorType = _sensorType;
	pin = _sensorPin;
	lastTime = 0;
}

// Sort an array
void SharpIR::sort(int a[], int size) {
	for (int i = 0; i < (size - 1); i++) {
		bool flag = true;
		for (int o = 0; o < (size - (i + 1)); o++) {
			if (a[o] > a[o + 1]) {
				int t = a[o];
				a[o] = a[o + 1];
				a[o + 1] = t;
				flag = false;
			}
		}
		if (flag) break;
	}
}

double  SharpIR::getDistance()
{
	return this->getDistance(true);
}

double  SharpIR::getDistance(bool avoidBurstRead)
{
	if (!avoidBurstRead) while (millis() <= lastTime + 20) {} //wait for sensor's sampling time


	lastTime = millis();

	int NB_SAMPLE = 20;
	int ir_val[NB_SAMPLE];

	double mode = 0.0;

	double something = 0.0;
	double exponent = 0.0;
	double numerator = 0.0;
	double cube = 0.0;
	double square = 0.0;
    double test = 0.0;
    double test1 = 0.0;
	// read 50
	for (int i = 0; i < NB_SAMPLE; i++)
	{
		// Read analog value
		ir_val[i] = analogRead(pin);
	}

	sort(ir_val, NB_SAMPLE);

	//mode = zhenYongLikesItDifficult();

	switch (sensorType)
	{
	case GP2YA41SK0F:

		distance = 2076 / (ir_val[NB_SAMPLE / 2] - 11);

		if (distance > 30) return 31;
		else if (distance < 4) return 3;
		else return distance;

		break;

	//Short range sensor
	case GP2Y0A21YK0F:
		switch (pin) {
			case A0:
				something = ir_val[NB_SAMPLE / 2] / 1;
                exponent = 65.9035675127*exp(-0.0055169752*something);
                return exponent;
				break;

			case A1:
                something = ir_val[NB_SAMPLE / 2] / 1;
                exponent = pow(something, -1.584);
                return (76234*exponent);
				break;

			case A2:
				something = ir_val[NB_SAMPLE / 2] / 1;
                exponent = 66.1481105360*exp(-0.0055218123*something);
                return exponent;
				break;


			case A3:
				something = ir_val[NB_SAMPLE / 2] / 1;
                exponent = 114.1618947497*exp(-0.0079304803*something);
                return exponent;
				break;

			case A4:
				something = ir_val[NB_SAMPLE / 2] / 1;
                exponent = 106.9475403057*exp(-0.0078093589*something);
                return exponent;
				break;

			case A5:
                something = ir_val[NB_SAMPLE / 2] / 1;
                exponent = 106.5558121623*exp(-0.0078083772*something);
                return (exponent);
                break;
		}
		break;

	//Long range sensor
	case GP2Y0A02YK0F:
        something = ir_val[NB_SAMPLE / 2] / 1;
        cube = pow(something, 3);
        exponent = -2*pow(10, -6);
        square = 0.0025*pow(something, 2);
        return (cube*exponent+square-1.1207*something+195.48);
        break;


	}
}
