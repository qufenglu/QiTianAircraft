/*
 * PostionPIDCtr.h
 *
 *  Created on: 2022Äê2ÔÂ14ÈÕ
 *      Author: Qufenglu
 */

#ifndef MAIN_DRIVER_POSTIONPIDCTR_H_
#define MAIN_DRIVER_POSTIONPIDCTR_H_
#include "stdint.h"

class PostionPIDCtr
{
public:
	typedef struct PID
	{
		float fP;
		float fI;
		float fD;
	} PID;

	typedef struct Error
	{
		float fCurrentError = .0f;
		float fLastError = .0f;
		float fErrorSum = .0f;
		float fErrorMax;
		float fErrorMin;
	} Error;
public:
	PostionPIDCtr();
	virtual ~PostionPIDCtr();
	void Init(const PostionPIDCtr::PID &pid, float fErrorMin, float fErrorMax);
	float CalPIDOut(const float &fTarget, const float &fMeasure, const int32_t &dertT, const float &fMaxOut);
	void Reset();

private:
	PostionPIDCtr::PID m_PID;
	PostionPIDCtr::Error m_Error;
};

#endif /* MAIN_DRIVER_POSTIONPIDCTR_H_ */
