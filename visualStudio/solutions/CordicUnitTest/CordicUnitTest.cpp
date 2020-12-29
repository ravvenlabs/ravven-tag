#include "CppUnitTest.h"
#include <iostream>
#include "CordicAtan2.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define MAX_THETA_THRESHOLD_ERROR 0.001f
#define MAX_MAG_THRESHOLD_ERROR 0.0001f

namespace CordicUnitTest
{
	TEST_CLASS(CordicUnitTest)
	{
	public:
		wchar_t outputBufferWC[128];

		TEST_METHOD(CordicAngleTest)
		{
			float Ix;
			float Iy;
			float diff;

			float cordicTheta;
			float gnuTheta;

			for (int x = -100; x <= 100; x++)
			{
				Ix = x * 0.01f;
				for (int y = -100; y <= 100; y++)
				{
					Iy = y * 0.01f;
					cordicTheta = cordicAtan2(Iy, Ix, lut12).second;
					gnuTheta = atan2f(Iy, Ix);

					diff = abs(cordicTheta - gnuTheta);

					swprintf(outputBufferWC, 128, L"Error too large: %0.4f.\n"
						L"X: %0.4f\tY: %0.4f\n"
						L"Cordic: %0.4f\nGNU: %0.4f",
						diff, Ix, Iy, cordicTheta, gnuTheta);
					Assert::IsTrue(diff < MAX_THETA_THRESHOLD_ERROR, outputBufferWC);
				}
			}
		}

		TEST_METHOD(CordicMagTest)
		{
			float Ix;
			float Iy;
			float diff;

			float cordicMag;
			float gnuMag;

			for (int x = -100; x <= 100; x++)
			{
				Ix = x * 0.01f;
				for (int y = -100; y <= 100; y++)
				{
					Iy = y * 0.01f;
					cordicMag = cordicAtan2(Iy, Ix, lut12).first;
					gnuMag = sqrt(Iy * Iy + Ix * Ix);

					diff = abs(cordicMag - gnuMag);

					swprintf(outputBufferWC, 128, L"Error too large: %0.4f.\n"
						L"X: %0.4f\tY: %0.4f\n"
						L"Cordic: %0.4f\nGNU: %0.4f",
						diff, Ix, Iy, cordicMag, gnuMag);
					Assert::IsTrue(diff < MAX_MAG_THRESHOLD_ERROR, outputBufferWC);
				}
			}
		}
	};
}
