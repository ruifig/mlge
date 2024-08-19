#include "mlgegen_custom.h"

#include <stdlib.h>
#include <stdio.h>

namespace mlge::gen
{

using namespace cz;

void test1()
{
	test2();
}

}

namespace cz
{
void test2()
{
	printf("Default implementation\n");
}
}



