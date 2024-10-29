#include "pch.h"
#include "CppUnitTest.h"
#include "../TSS/Library.h"
#include "Images.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestTSS
{
	TEST_CLASS(Histogram)
	{
	public:
		
		TEST_METHOD(FirstTest)
		{
			//--------------B--G--R--A---B---G---R--A
			BYTE data[] = { 2, 4, 6, 0, 10, 12, 14, 0 };

			std::vector<UINT> red;
			std::vector<UINT> green;
			std::vector<UINT> blue;

			loop(_, 256)
			{
				red.push_back(0);
				green.push_back(0);
				blue.push_back(0);
			}

			::CalcHist(reinterpret_cast<UINT*>(data), 1, 1, 2, &red, &green, &blue);

			std::vector<UINT> expected_red;
			std::vector<UINT> expected_green;
			std::vector<UINT> expected_blue;

			loop(i, 256)
			{
				if (i == 6) expected_red.push_back(255);
				else if (i == 14) expected_red.push_back(255);
				else expected_red.push_back(0);

				if (i == 4) expected_green.push_back(255);
				else if (i == 12) expected_green.push_back(255);
				else expected_green.push_back(0);

				if (i == 2) expected_blue.push_back(255);
				else if (i == 10) expected_blue.push_back(255);
				else expected_blue.push_back(0);
			}

			Assert::IsTrue(expected_red == red);
			Assert::IsTrue(expected_green == green);
			Assert::IsTrue(expected_blue == blue);
		}

		TEST_METHOD(OneSquare200x200)
		{
			std::vector<UINT> red;
			std::vector<UINT> green;
			std::vector<UINT> blue;

			loop(_, 256)
			{
				red.push_back(0);
				green.push_back(0);
				blue.push_back(0);
			}

			::CalcHist(reinterpret_cast<UINT*>(gpiOneSquare200x200), 200, 200, 200, &red, &green, &blue);

			std::vector<UINT> expected_red;
			std::vector<UINT> expected_green;
			std::vector<UINT> expected_blue;

			loop(i, 256)
			{
				if (i == 50) expected_red.push_back(44);
				else if (i == 200) expected_red.push_back(255);
				else expected_red.push_back(0);

				if (i == 60) expected_green.push_back(44);
				else if (i == 100) expected_green.push_back(255);
				else expected_green.push_back(0);

				if (i == 50) expected_blue.push_back(255);
				else if (i == 70) expected_blue.push_back(44);
				else expected_blue.push_back(0);
			}

			Assert::IsTrue(expected_red == red);
			Assert::IsTrue(expected_green == green);
			Assert::IsTrue(expected_blue == blue);
		}

		TEST_METHOD(TwoCircles100x100)
		{
			std::vector<UINT> red;
			std::vector<UINT> green;
			std::vector<UINT> blue;

			loop(_, 256)
			{
				red.push_back(0);
				green.push_back(0);
				blue.push_back(0);
			}

			::CalcHist(reinterpret_cast<UINT*>(gpiTwoCircles100x100), 100, 100, 100, &red, &green, &blue);

			std::vector<UINT> expected_red;
			std::vector<UINT> expected_green;
			std::vector<UINT> expected_blue;

			loop(i, 256)
			{
				if (i == 70) expected_red.push_back(255);
				else if (i == 80) expected_red.push_back(55);
				else if (i == 128) expected_red.push_back(101);
				else expected_red.push_back(0);

				if (i == 128) expected_green.push_back(101);
				else if (i == 140) expected_green.push_back(255);
				else if (i == 160) expected_green.push_back(55);
				else expected_green.push_back(0);

				if (i == 128) expected_blue.push_back(101);
				else if (i == 210) expected_blue.push_back(255);
				else if (i == 240) expected_blue.push_back(55);
				else expected_blue.push_back(0);
			}

			Assert::IsTrue(expected_red == red);
			Assert::IsTrue(expected_green == green);
			Assert::IsTrue(expected_blue == blue);
		}

		TEST_METHOD(NightHouse150x150)
		{
			std::vector<UINT> red;
			std::vector<UINT> green;
			std::vector<UINT> blue;

			loop(_, 256)
			{
				red.push_back(0);
				green.push_back(0);
				blue.push_back(0);
			}

			::CalcHist(reinterpret_cast<UINT*>(gpiNightHouse150x150), 150, 150, 150, &red, &green, &blue);

			std::vector<UINT> expected_red;
			std::vector<UINT> expected_green;
			std::vector<UINT> expected_blue;

			loop(i, 256)
			{
				if (i == 10) expected_red.push_back(255);
				else if (i == 150) expected_red.push_back(85);
				else if (i == 240) expected_red.push_back(16);
				else expected_red.push_back(0);

				if (i == 10) expected_green.push_back(255);
				else if (i == 120) expected_green.push_back(85);
				else if (i == 240) expected_green.push_back(16);
				else expected_green.push_back(0);

				if (i == 0) expected_blue.push_back(16);
				else if (i == 10) expected_blue.push_back(255);
				else if (i == 90) expected_blue.push_back(85);
				else expected_blue.push_back(0);
			}

			Assert::IsTrue(expected_red == red);
			Assert::IsTrue(expected_green == green);
			Assert::IsTrue(expected_blue == blue);
		}
	};
}
