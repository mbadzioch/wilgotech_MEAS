#ifndef MAIN_H_
#define MAIN_H_


typedef struct{
	uint8_t seconds;     /*!< Seconds parameter, from 00 to 59 */
	uint8_t minutes;     /*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;       /*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t date;        /*!< Date in a month, 1 to 31 */
	uint8_t month;       /*!< Month in a year, 1 to 12 */
	uint8_t year;        /*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	uint32_t hms;		 //Seconds+Minutes+Hours in sec
}sTimeS;

sTimeS sTime;

char cbuf[128];
#endif /* MAIN_H_ */
