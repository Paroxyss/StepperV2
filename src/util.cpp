#include <config.h>
#include <Arduino.h>
#include <sstream>
#include <SPIFFS.h>
#include <MD5Builder.h>

using namespace std;

void step(const uint8_t *motorPins, uint8_t step, boolean halfStep)
{
	if ((!halfStep) && step % 2 == 0)
		return;
	for (int i = 0; i < 4; i++)
	{
		digitalWrite(motorPins[i], states[step][i]);
	}
}

String toString(int number)
{
	string str; // a variable of str data type

	// using the stringstream class to insert an int and
	// extract a string
	stringstream ss;
	ss << number;
	ss >> str;
	return String(str.c_str());
}

int getFirstDispoId()
{
	int maxId = 0;
	while (SPIFFS.exists("/" + toString(maxId)))
	{
		maxId++;
	}
	return maxId;
}

int getLastId(){
	int maxId = 0;
	while (SPIFFS.exists("/" + toString(maxId)))
	{
		maxId++;
	}
	maxId++;
	while (SPIFFS.exists("/" + toString(maxId)))
	{
		maxId++;
	}
	return maxId - 1;
}

boolean deleteFile(int id)
{
	String path = "/" + toString(id);
	if (SPIFFS.exists(path))
	{
		SPIFFS.remove(path);

		// now we have to rename the file with the bigger id to fill the gap
		int maxId = getLastId();

		if (maxId <= id)
		{ // we deleted the last file
			return true;
		}

		String movePath = "/" + toString(maxId);

		Serial.printf("renaming %s to %s after delete\n", movePath, path);

		SPIFFS.rename(movePath, path);

		return true;
	}
	return false;
}

String file_md5(File &f)
{
	if (!f)
	{
		return String();
	}

	if (f.seek(0, SeekSet))
	{

		MD5Builder md5;
		md5.begin();
		md5.addStream(f, f.size());
		md5.calculate();
		return md5.toString();
	}
	return String();
}