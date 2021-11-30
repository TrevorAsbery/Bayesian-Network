#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <math.h>

using namespace std;

//process words in the text files
string ProcessWord(string s)
{
	string t;

	// Remove punctuation.
    for (int i = 0; i < s.size(); i++)
        if (!ispunct(s[i]))
			t += s[i];

	// Convert to lower case.
	std::transform(t.begin(), t.end(), t.begin(), ::tolower);
	
	return t;
}

//parse the training file and build the map that contains dictionary words and their counter for each class
void ParseTrainingFile(string filename, int c, map<string, vector<int> > & dict)
{
	// Open file.
	ifstream in;
	in.open(filename.c_str());
	set<string> wordsCounted;
	if (!in.is_open())
	{
		cout<<"File not found: "<<filename<<endl;
		return;
	}
	
	// Find the end of the header.
	string line;
	while (getline(in, line))
	{
		if (line == "")
			break;
	}

	// Read the rest of the file word by word.
	string word;
	while (in >> word)
	{
		// Strip the word of punctuation and convert to lower case.
		word = ProcessWord(word);
		
		if (word != "")
		{		
			//if the word in the file is in the dictionary and has not been counted for this file yet
			if(dict.find(word)!=dict.end() && wordsCounted.find(word)==wordsCounted.end()){

				dict[word][c]++;
				wordsCounted.insert(word);
			}
			//cout<<word<<endl;
		}
	}
}

//parse the test file and return the set of words from the test file that exist in the dictionary
set<string> ParseTestFile(string filename, int c, map<string, vector<int> > & dict)
{
	// Open file.
	ifstream in;
	in.open(filename.c_str());
	set<string> words;
	if (!in.is_open())
	{
		cout<<"File not found: "<<filename<<endl;
		return words;
	}
	
	// Find the end of the header.
	string line;
	while (getline(in, line))
	{
		if (line == "")
			break;
	}

	// Read the rest of the file word by word.
	string word;
	while (in >> word)
	{
		// Strip the word of punctuation and convert to lower case.
		word = ProcessWord(word);
		
		if (word != "")
		{		
			//if the word in the file is in the dictionary and has not been counted for this file yet
			if(dict.find(word)!=dict.end()){
				words.insert(word);
			}
		}
	}

	return words;
}

// calculate P(cm=c)
double PofMessageInC(int c, vector<int> &totalC, int total_messages){
	return ((double)totalC[c]/(total_messages));
}

//calculate P(x=true|cm=c) Note take one minus this function to get P(x=false|cm=c) = 1-P(x=true|cm=c)
double PofMessageHasWgivenInC(string w, int c, vector<int>& totalC, map<string, vector<int> >& dict){

	double numerator = dict[w][c]+1;
	double denominator = totalC[c]+2;
	
	return(numerator/denominator);

}

//main classifies the files in test_list
int main()
{

	//build the dictionary
	ifstream dictionary;
	dictionary.open("dictionary");
	map<string, vector<int> > dict;
	string dict_word;
	vector<string> dict_list;

	//read in words in the dictionary and set all of the class values to zero to start
	while(dictionary>>dict_word){
		vector<int> temp;
		temp.push_back(0); //c0
		temp.push_back(0); //c1
		temp.push_back(0); //c2
		temp.push_back(0); //c3
		dict.insert(make_pair(dict_word, temp));

		//make a dict_list so that we can recall the dictionary items in order later
		dict_list.push_back(dict_word);
	}
	dictionary.close();

	//parse the training list to find the number of total messages in each class
	ifstream training_list;
	training_list.open("training_list");
	string title;
	int c;

	//vector containing the total messages in each class
	vector<int> totalCs;
	totalCs.push_back(0); //c0
	totalCs.push_back(0); //c1
	totalCs.push_back(0); //c2
	totalCs.push_back(0); //c3

	// total messages
	int totalMessages = 0;

	//go through the training list, increment the class and total messages
	//also parese the file to fill in the map inialiazed above
	while(training_list>>title){
		training_list>>c;

		totalCs[c]++;
		totalMessages++;

		//decide which words from dict are in title
		ParseTrainingFile(title, c, dict);

	}
	training_list.close();

	//build the network.txt file
	ofstream network;
	network.open("network.txt");

	//print the dictionary words and print the class and P(x=true|cm = c) for each word
	for(int i=0; i<dict_list.size(); i++){
		for(int j=0; j<4; j++){
			network<<dict_list[i]<<"\t"<<j<<"\t"<<PofMessageHasWgivenInC(dict_list[i], j, totalCs, dict);
			network<<endl;
		}
	}

	network.close();

	//create input file stream for test file
	ifstream test;
	test.open("test_list");

	//create output file stream for classification.txt
	ofstream classification;
	classification.open("classification.txt");

	//string for the testfile
	string testfile;

	//the true probability of the class
	int real_class;

	//loop through every testfile
	while(test>>testfile){
		test>>real_class;

		double maxP = INT_MIN;
		int prediction = 6969;

		//parse the test file and return the set of words that exist
		//in both the message and the dictionary
		set<string> temp = ParseTestFile(testfile, c, dict);

		//for each of the four classes, do the algorithm from the pdf
		for(int c=0; c<4; c++){
			
			//probability summation starts at 0
			double prob_ci=0;

			//for all words in the dictionary
			for(auto it=dict.begin(); it!=dict.end(); it++){
				
				//if the word exists in the message add log(P(x=true|cm=c)) to the summation
				if(temp.find(it->first) != temp.end()){

					prob_ci += log(PofMessageHasWgivenInC(it->first, c, totalCs, dict));

				}

				//else if the word does not exist in the message add log(P(x=false|cm=c)) to the summation
				else{

					prob_ci += log(1-PofMessageHasWgivenInC(it->first, c, totalCs, dict));

				}

			}

			//finally add the log(P(cm=c)) to the summation
			prob_ci = prob_ci + log(PofMessageInC(c, totalCs, totalMessages));

			//if the probability of this class is greater than the current max
			//make it the new max
			if(maxP<prob_ci){
				maxP = prob_ci;
				prediction = c;
			}
	
		}

		//print the name of the test file, the actual classification, 
		//and the predicted classification
		classification<<testfile<<"\t"<<real_class<<"\t"<<prediction<<endl;
		
	}

	//inialize matrix for classification-summary.txt
	int matrix[4][4]; 

	//create an output stream for classification-summary.txt
	ofstream class_sum;
	class_sum.open("classification-summary.txt");

	//read in classification.txt to create classification-sum.txt
	ifstream readin;
	readin.open("classification.txt");
	string trash = "";

	//initialize row and col
	int row = 0;
	int col = 0;

	//inialize the matrix with 0's
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){

			matrix[i][j]=0;
		}
	}

	//trash is the name of the file and is not necessary for the matrix
	while(readin>>trash){
		readin>>row;
		readin>>col;

		//increment the value of row = class, col = predicted class
		matrix[row][col]++;
	}

	//print the matrix to classification-summary.txt
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){

			class_sum<<matrix[i][j]<<"\t";
		}
		class_sum<<endl;
	}


	return 0;
}
