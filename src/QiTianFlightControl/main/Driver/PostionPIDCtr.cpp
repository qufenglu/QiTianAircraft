/*
 * PostionPIDCtr.cpp
 *
 *  Created on: 2022Äê2ÔÂ14ÈÕ
 *      Author: Qufenglu
 */

#include <math.h>
#include "PostionPIDCtr.h"

PostionPIDCtr::PostionPIDCtr()
{

}

PostionPIDCtr::~PostionPIDCtr()
{

}

void PostionPIDCtr::Init(const PostionPIDCtr::PID &pid, float fErrorMin, float fErrorMax)
{
	m_PID.fP = pid.fP;
	m_PID.fI = pid.fI;
	m_PID.fD = pid.fD;

	m_Error.fCurrentError = .0f;
	m_Error.fErrorSum = .0f;
	m_Error.fLastError = .0f;

	m_Error.fErrorMin = fErrorMin;
	m_Error.fErrorMax = fErrorMax;
}

float PostionPIDCtr::CalPIDOut(const float &fTarget, const float &fMeasure, const int32_t &dertT, const float &fMaxOut)
{
	float dt= dertT/1000000.0f;
	m_Error.fCurrentError = fTarget - fMeasure;
	float out = (m_PID.fP * m_Error.fCurrentError)
			  + (m_PID.fI * m_Error.fErrorSum)
			  + (m_PID.fD * (m_Error.fCurrentError - m_Error.fLastError)/dt);
	m_Error.fLastError = m_Error.fCurrentError;

	if(fabs(out) < fMaxOut)
	{
		float temp = m_Error.fErrorSum + (m_Error.fCurrentError * dt);
		if(temp > m_Error.fErrorMin && temp < m_Error.fErrorMax)
		{
			m_Error.fErrorSum = temp;
		}
	}
	else
	{
		out = out < 0 ? (0 - fMaxOut) : fMaxOut;
	}

	return out;
}

void PostionPIDCtr::Reset()
{
	m_Error.fCurrentError = .0f;
	m_Error.fErrorSum = .0f;
	m_Error.fErrorSum = .0f;
}
