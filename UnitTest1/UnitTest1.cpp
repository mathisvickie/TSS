#include "pch.h"
#include "CppUnitTest.h"
#include "../TSS/Library.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestNameSpace
{
	TEST_CLASS(UnitTestClass)
	{
	public:
		
		TEST_METHOD(TestMona60x90)
		{
			BYTE data[] = {2, 4, 6, 0, 10, 12, 14, 0};

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

			int x = 1;
		}
	};
}
