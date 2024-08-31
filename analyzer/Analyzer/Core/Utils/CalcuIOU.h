#ifndef ANALYZER_CALCUIOU_H
#define ANALYZER_CALCUIOU_H

#include <vector>

namespace AVSAnalyzer {

	/*
	调用案例

	double p[8] = { 0, 0, 0, 80, 300, 80, 300, 0 }; // x1,y1,x2,y2,x3,y3,x4,y4
	double q[8] = { 0, 40, 0, 120, 300, 120, 300, 40 };

	std::vector<double> P(p, p + 8);
	std::vector<double> Q(q, q + 8);
	double iou = CalcuPolygonIOU(P, Q);
	*/
	double CalcuPolygonIOU(std::vector<double> p, std::vector<double> q);
	
}
#endif //ANALYZER_CALCUIOU_H