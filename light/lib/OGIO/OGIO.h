
class OGIO {
public:
    char *nodeTag;

	void initR(char *nodeTag);

	int getPhotoResistorADC();
	int PhotoResistorADC;

	float getBH1750LuxLevel();
	float BH1750LuxLevel;

	void activateLightRelay();
	void deactivateLightRelay();

	String generateInfluxLineProtocol();
};
