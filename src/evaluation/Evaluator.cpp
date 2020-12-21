#include "Evaluator.h"
#include <cstdlib>

int Evaluator::evaluate() {
	return rand() & 63;
}