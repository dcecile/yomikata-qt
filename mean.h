#ifndef MEAN_H
#define MEAN_H

class Mean
{
public:
	Mean(int saturation, int restart);
	~Mean();

	void addSample(double value);
	double value() const;

private:
	double _mean;
	int _samples;
	int _saturation;
	int _restart;
};

#endif
