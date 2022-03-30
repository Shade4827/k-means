#include <iostream>
#include <vector>
#include <random>
#include <array>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

vector<array<int, 2>> GeneratePoints(size_t, int, int);
vector<array<double, 2>> GetTypicalPoints(int, vector<array<int, 2>>);
vector<vector<array<int, 2>>> ClassifyCluster(vector<array<int, 2>>, vector<array<double, 2>>);
vector<array<double, 2>> RecalcTypicalPoint(vector<vector<array<int, 2>>>);
void ShowTypicalPoints(vector<array<double, 2>>);

int main() {
	int clusterNum;
	int count = 0;
	vector<array<int,2>> points;
	vector<array<double,2>> typicalPoints;
	vector<array<double, 2>> nextTypicalPoints;
	vector<vector<array<int,2>>> cluster;

	//クラスター個数の決定
	do {
		cout << "クラスターの個数:";
		cin >> clusterNum;
	} while (clusterNum <= 0);
	cout << clusterNum << endl;

	//点の生成
	points = GeneratePoints(100, -100, 100);

	//代表点の決定
	typicalPoints = GetTypicalPoints(clusterNum, points);
	
	while (true) {
		count++;
		cout << count << ":";
		ShowTypicalPoints(typicalPoints);

		//代表点との距離を比較して一番近いクラスターに分別
		cluster = ClassifyCluster(points, typicalPoints);

		//クラスター点が決定したら代表点を再計算
		nextTypicalPoints = RecalcTypicalPoint(cluster);

		//計算結果に基づいて分別を繰り返す
		//代表点が移動しなくなったら終了
		if (typicalPoints == nextTypicalPoints) {
			break;
		}
		else {
			typicalPoints = nextTypicalPoints;
		}
	}

	//csvに出力
	ofstream ofs("output.csv");
	int max = 0;
	for (const auto &cp : cluster) {
		if (max < cp.size()) {
			max = cp.size();
		}
	}
	
	for (int i = 0; i < max; i++) {
		string str = "";
		for (int j = 0; j < cluster.size();j++) {
			if (cluster[j].size() > i) {
				str += to_string(cluster[j][i][0]) + "," + to_string(cluster[j][i][1]) + ",";
			}
			else {
				str += ",,";
			}
		}
		ofs << str << endl;
	}
}

/* ====================================================================== */
/**
 * @brief  min以上max未満の範囲の乱数値による二次元座標をsize個生成したリストを返す
 *
 * @param[size_t] size  生成する乱数の個数
 * 
 * @param[int] min  乱数の最小値
 * 
 * @param[int] max  乱数の最大値
 *
 * @return  二次元座標のリスト(vector<array<int,2>>)
 */
 /* ====================================================================== */
vector<array<int, 2>> GeneratePoints(size_t size, int min, int max) {
	vector<array<int, 2>> points;
	mt19937 mt(0);
	uniform_int_distribution<uint64_t> rnd(0, max + 1 - min);

	for (int i = 0; i < size; i++) {
		array<int, 2> temp = { rnd(mt) + min,rnd(mt) + min };
		bool equalFlag = false;
		for (const auto& p : points) {
			if (p == temp) {
				equalFlag = true;
				break;
			}
		}

		if (equalFlag) {
			i--;
			continue;
		}

		points.push_back(temp);
	}

	return points;
}


/* ====================================================================== */
/**
 * @brief  pointsの中からランダムにclusterNum個のクラスターの代表点を選ぶ
 *
 * @param[int] clusterNum  クラスターの個数
 *
 * @param[vector<array<int,2>>] points  二次元座標のリスト 
 *
 * @return  代表点のリスト(vector<array<double,2>>)
 */
 /* ====================================================================== */
vector<array<double, 2>> GetTypicalPoints(int clusterNum, vector<array<int, 2>> points) {
	vector<int> previousList;
	vector<array<double, 2>> typicalPoints;
	random_device rnd;
	mt19937 mt(rnd());

	for (int i = 0; i < clusterNum; i++) {
		size_t index = mt() % points.size();
		bool equalFlag = false;
		for (const auto& p : previousList) {
			if (p == index) {
				equalFlag = true;
				break;
			}
		}

		if (equalFlag) {
			i--;
			continue;
		}

		array<double, 2> temp = { points[index][0],points[index][1] };
		typicalPoints.push_back(temp);
		previousList.push_back(index);
	}

	return typicalPoints;
}


/* ====================================================================== */
/**
 * @brief  pointsを最も近いtypicalPointsのクラスターに分類する
 *
 * @param[vector<array<int,2>>] points  二次元座標のリスト
 *
 * @param[vector<array<double,2>>] typicalPoints  代表点のリスト
 *
 * @return  クラスターに分類された二次元座標のリスト(vector<vector<array<int,2>>>)
 */
 /* ====================================================================== */
vector<vector<array<int, 2>>> ClassifyCluster(vector<array<int, 2>> points, vector<array<double, 2>> typicalPoints) {
	vector<vector<array<int, 2>>> cluster(typicalPoints.size(), vector<array<int, 2>>(0, array<int, 2>()));

	for (const auto& p : points) {
		vector<double> distanceList;
		for (const auto& tp : typicalPoints) {
			double distance = sqrt(pow(p[0] - tp[0], 2) + pow(p[1] - tp[1], 2));
			distanceList.push_back(distance);
		}

		vector<double>::iterator iter = min_element(distanceList.begin(), distanceList.end());
		size_t index = distance(distanceList.begin(), iter);

		cluster[index].push_back(p);
	}

	return cluster;
}


/* ====================================================================== */
/**
 * @brief  clusterから代表点を再計算する
 *
 * @param[vector<vector<array<int,2>>] cluster  クラスターに分類された二次元座標のリスト
 *
 * @return  代表点のリスト(vector<array<double,2>>)
 */
 /* ====================================================================== */
vector<array<double, 2>> RecalcTypicalPoint(vector<vector<array<int, 2>>> cluster) {
	vector<array<double, 2>> typicalPoints;

	for (const auto& cp : cluster) {
		array<double, 2> center = { 0,0 };
		for (const auto& p : cp) {
			center[0] += p[0];
			center[1] += p[1];
		}
		center[0] /= cp.size();
		center[1] /= cp.size();
		typicalPoints.push_back(center);
	}

	return typicalPoints;
}


/* ====================================================================== */
/**
 * @brief  typicalPointsを表示する
 *
 * @param[vector<array<double,2>>] typicalPoints  代表点のリスト
 */
 /* ====================================================================== */
void ShowTypicalPoints(vector<array<double, 2>> typicalPoints) {
	for (const auto& tp : typicalPoints) {
		cout << "[" << tp[0] << "," << tp[1] << "] ";
	}
	cout << endl;
}