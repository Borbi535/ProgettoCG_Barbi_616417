#pragma once

/// <summary>
/// Teorema di Pitagora, se hypotenuse = true, allora il parametro l1 deve essere l'ipotenusa
/// </summary>
double pythagoras(double l1, double l2, bool hypotenuse = false)
{
	assert(l1 > 0 && l2 > 0);

	if (hypotenuse)
	{
		assert(l1 > l2);

		return glm::sqrt(pow(l1, 2) - pow(l2, 2));
	}

	return glm::sqrt(pow(l1, 2) + pow(l2, 2));
}

/// <summary>
/// Formula di Erone
/// </summary>
double hero(double l1, double l2, double l3)
{
	assert(l1 > 0 && l2 > 0 && l3 > 0);

	double p = (l1 + l2 + l3) / 2;
	return glm::sqrt(p * (p - l1) * (p - l2) * (p - l3));
}