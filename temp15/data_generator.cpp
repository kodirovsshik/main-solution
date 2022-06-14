
import libksn.time;
import <iostream>;
import <sstream>;

#pragma warning(disable : 4996)

int main()
{
	static constexpr size_t N = 100000;
	
	int dtms = 5;
	float measurements[N]{};
	float dt = dtms * 1e-3f;

	for (int i = 0; i < 20; ++i)
		ksn::sleep_for(dt);

	for (float& x : measurements)
	{
		ksn::stopwatch sw;
		sw.start();
		ksn::sleep_for(ksn::time(dt));
		x = sw.stop();
	}

	std::ostringstream ss;
	ss << "data " << dtms << "ms.bin";
	FILE* f = fopen(ss.str().c_str(), "wb");
	fwrite(measurements, 1, sizeof(measurements), f);
}
