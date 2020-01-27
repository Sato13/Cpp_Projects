// Andrew Knapp 
// KNN MPI
// Due: 03/25/19

#include <mpi.h>

#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace std;

bool comparison(vector<int> v1, vector<int> v2) {
	return(v1[0] < v2[0]);
}

bool compare(int i1, int i2) {
	return(i1 < i2);
}

void print_vector(vector<int> vec) {
	for (int z = 0; z <= vec.size() - 1; z++) {
		cout << vec[z];
	}
}
vector< vector<int> > file_struct(string path, int getclass) {

	ifstream in(path);
	string line;

	// input file then skip first line
	getline(in, line);

	int index = 0;
	string value;
	vector< vector<int> > result;


	while (getline(in, line)) {
		vector<int> splitline;

		while ((index = line.find(",")) != string::npos) {
			value = line.substr(0, index);
			splitline.push_back(stoi(value));
			line.erase(0, index + 1);
		}

		if (getclass == 1) {
			value = line.substr(0, index);
			splitline.push_back(stoi(value));
			line.erase(0, index + 1);
		}
		result.push_back(splitline);
	}

	in.close();
	return result;
}

int output(vector< vector<int> > distV, int k) {
	sort(distV.begin(), distV.end(), comparison);

	vector<int> classes;

	for (int i = 0; i < k; i++) {
		vector<int> current = distV[i];
		classes.push_back(current[1]);
	}
	// ascending order
	sort(classes.begin(), classes.end(), compare);
	int max_count = 0;
	int current_count = 0;
	int max_class = -1;

	// go through list to fide matches in clasees
	for (int i = 0; i < classes.size() - 1; i++) {
		if (classes[i] == classes[i + 1]) {
			current_count++;
		} else if (current_count > max_count) {
			max_class = classes[i];
			max_count = current_count;
			current_count = 0;
		}
	}

	// check if the count of the last class in the list is greater or not
	if (current_count > max_count) {
		max_class = classes[classes.size() - 1];
	}

	// send back the class that was most frequent
	return max_class;
}

int main()
{
	int	threads = 12;
	int	rank;
	int k = 13; // change k for how many closest results wanted

	cout << threads;


	string testfile = "/home/shared/benton_class/Data/Covertype/Covertype.test";
	string train = "/home/shared/benton_class/Data/Covertype/Covertype.train";
	string line;
	size_t index = 0;
	string value;

	vector< vector<int> > testV;
	vector< vector<int> > trainV;
	testV = file_struct(testfile, 1);
	trainV = file_struct(train, 0);

	// divide list to processors
	int	batch = trainV.size() / (threads - 1);

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &threads);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// populate list
	if (rank != 0) {
		int start = (rank - 1) * batch;
		int finish = (rank * batch) - 1;
		int tr_size = trainV.size() - 1;
		int tst_size = testV.size() - 1;

		if (finish > tr_size) {
			finish = tr_size;
		}

		cout << rank << " - " << start << endl;
		cout << finish << endl;

		// iterate through trainV
		for (int i = start; i <= finish; i++) {

			vector<int> curTrain = trainV[i];
			print_vector(curTrain);

			vector< vector<int> > range;
			int perfect_matches = 0;

			// iterate through each line of test data
			for (int j = 0; j <= tst_size; j++) {
				vector<int> current_test = testV[j];
				int dist = 0;

				for (int z = 0; z <= curTrain.size() - 1; z++) {
					if (curTrain[z] != current_test[z]) {
						dist++;
						// dist += sqrt(pow(abs(current_test[i] - curTrain[i]), 2));
						// above for Iris, switch with if statement
					}
				}

				if (dist == 0) {
					perfect_matches++;
				}

				if (perfect_matches == k) {
					break;
					perfect_matches = 0;
				}

				int x = current_test[current_test.size() - 1];
				
				vector<int> dist_class;
				
				dist_class.push_back(dist);
				dist_class.push_back(x);
				range.push_back(dist_class);

			}

			print_vector(curTrain);
			cout << " = class " << std::to_string(output(range, k)) << ", rank: " << std::to_string(rank) << "\n";
		}
	}

	MPI_Finalize();
}

