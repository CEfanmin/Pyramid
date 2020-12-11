#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
int set_decibel()
{
    ifstream in_file( "/home/fanmin/catkin_ws/src/hori_micphone/set_decibel.txt"  );
    if ( ! in_file )
       { cerr << "oops! unable to open input file\n"; return -1; }

	int val;
	vector< int > text;

    while ( in_file >> val )
	{
    	text.push_back(val);
	}
	int ix;
	for ( ix = 0; ix < text.size(); ++ix )
		{
			cout << text[ix] <<"\n";
			if (text[ix] == 63)
			{
				cout <<"read is suceess\n";
			}
		}
	return text[0];
}
