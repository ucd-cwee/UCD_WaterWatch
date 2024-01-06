#pragma once

/**
 *
 * Changelog
 *  2008/04/25
 *  - added functions to read datasets in Liadan format
 *  - added functions to save centers in old format(separated by a ',') and Liadan format
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vector>
#include "vec.h"
#include "List.h"

static int KMeans_mti = 624 + 1;
static std::vector<unsigned long> KMeans_mt = std::vector<unsigned long>(624, 0.0); /* the array for the state vector  */
class KMeans {
public:
	static constexpr decltype(auto) N = 624;
	static constexpr decltype(auto) M = 397;
	static constexpr decltype(auto) MATRIX_A = 0x9908b0dfUL; /* constant vector a */
	static constexpr decltype(auto) UPPER_MASK = 0x80000000UL; /* most significant w-r bits */
	static constexpr decltype(auto) LOWER_MASK = 0x7fffffffUL; /* least significant r bits */
	static constexpr decltype(auto) THRESHOLD = 1.000; /* Determines when Lloyd terminates (should be between 0 and 1) */	

public:
	/** struct representing a single point **/
	struct point {
		//dimension
		int dimension;

		//Clustering Features (weight, squareSum, linearSum)
		double weight;
		double squareSum;
		float* coordinates;

		//cost and index of the centre, the point is currently assigned to
		double curCost;
		int centreIndex;


		//id and class (if there is class information in the file)
		int id;
		int cl;
	};

	/** datastructure representing a node within a tree **/
	struct treeNode {
		//number of points in this node
		int n;

		//array with pointers on points
		struct point** points;

		//pointer on the centre of the treenode
		struct point* centre;

		//pointer on the left childnode
		struct treeNode* lc;

		//pointer on the right childnode
		struct treeNode* rc;

		//pointer on the parent node
		struct treeNode* parent;

		//cost of the treenode
		double cost;
	};

	/** datastructure representing a single bucket **/
	struct Bucket {
		int cursize;
		struct point* points;
		struct point* spillover;
	};

	/** datastructure for managing all O(log(n)) buckets **/
	struct Bucketmanager {
		int numberOfBuckets;
		int maxBucketsize;
		struct Bucket* buckets;
	};

public:
	/** initializes a point **/
	static void initPoint(struct point* point, int dimension) {
		point->weight = 1.0;
		point->squareSum = 0.0;
		point->dimension = dimension;
		point->coordinates = (float*)malloc(dimension * sizeof(float));
		point->id = -1;
		point->cl = -1;
		point->curCost = 0;
		point->centreIndex = -1;
		int l = 0;
		for (l = 0; l < dimension; l++) {
			point->coordinates[l] = 0.0;
		}
	};

	/** deletes a point **/
	static void freePoint(struct point* point) {
		free(point->coordinates);
	};

	/** copys the data of the first point to the address of the second point **/
	static void copyPoint(struct point* firstPoint, struct  point* secondPoint) {
		initPoint(secondPoint, firstPoint->dimension);
		secondPoint->id = firstPoint->id;
		secondPoint->weight = firstPoint->weight;
		secondPoint->cl = firstPoint->cl;
		secondPoint->squareSum = firstPoint->squareSum;
		secondPoint->curCost = firstPoint->curCost;
		secondPoint->centreIndex = firstPoint->centreIndex;
		int l;
		for (l = 0; l < firstPoint->dimension; l++) {
			secondPoint->coordinates[l] = firstPoint->coordinates[l];
		}
	};

	/**
	copys the data of the first point to the address of the second point without initializing the second
	point first (this means, that the storage for the coordinates of the second point has to be already reserved)
	**/
	static void copyPointWithoutInit(struct point* firstPoint, struct  point* secondPoint) {
		secondPoint->id = firstPoint->id;
		secondPoint->weight = firstPoint->weight;
		secondPoint->cl = firstPoint->cl;
		secondPoint->squareSum = firstPoint->squareSum;
		secondPoint->curCost = firstPoint->curCost;
		secondPoint->centreIndex = firstPoint->centreIndex;
		int l;
		for (l = 0; l < firstPoint->dimension; l++) {
			secondPoint->coordinates[l] = firstPoint->coordinates[l];
		}
	};
	
	/**
	copys the data of the first point to the address of the second point without initializing the second
	point first (this means, that the storage for the coordinates of the second point has to be already reserved)
	**/
	static void copyPointWithoutInit(vec2d const& firstPoint, int index, struct point* secondPoint) {
		secondPoint->id = index;
		secondPoint->weight = 1.0;
		secondPoint->cl = -1;
		secondPoint->squareSum = firstPoint.LengthSqr();
		secondPoint->curCost = 0;
		secondPoint->centreIndex = -1;
		secondPoint->coordinates[0] = firstPoint.x;
		secondPoint->coordinates[1] = firstPoint.y;
	};


	static double readDouble(FILE* file) {
		char c;
		static char buf[100];

		int i = 1;
		while ((c = fgetc(file)) == ' ' && c != EOF) {}

		buf[0] = c;

		while ((c = fgetc(file)) != '\n' && c != ' ' && c != EOF) {
			buf[i++] = c;
		}
		buf[i] = 0;

		return atof(buf);
	};

	/** returns the next point of a file (Gereons format) **/
	static void getNextPointGereon(FILE* file, struct point* p, int dimension) {
		int l;
		for (l = 0; l < dimension; l++) {
			float nextNumber;
			char nextLine[256];
			fgets(nextLine, 256, file);
			sscanf(nextLine, "%f", &nextNumber);
			p->coordinates[l] = nextNumber;
			p->squareSum += nextNumber * nextNumber;
		}
		p->weight = 1.0;
	};
	/* Returns the next point of a file (Liadan format) */
	static void getNextPointLiadan(FILE* file, struct point* p, int dimension) {

		// skip first number
		readDouble(file);

		for (int l = 0; l < dimension; l++) {
			// read next number
			float nextNumber = readDouble(file);

			p->coordinates[l] = nextNumber;
			p->squareSum += nextNumber * nextNumber;

		}

		p->weight = 1.0;
	};

	/* saves the centers to a file with the separator ',' */
	static void saveCentersOldFormat(FILE* out, int numberOfCenters, struct point* centers, int dimension) {
		for (int i = 0; i < numberOfCenters; i++) {
			int l;
			for (l = 0; l < dimension - 1; l++) {
				// fprintf(out, "%f,", centers[i].coordinates[l] / centers[i].weight);
			}
			// fprintf(out, "%f", centers[i].coordinates[dimension - 1] / centers[i].weight);
			// fprintf(out, "\n");
		}
	};

	/* saves the centers to a file with the separator ' ' and a the weight as the first row */
	static void saveCentersLiadanFormat(FILE* out, int numberOfCenters, struct point* centers, int dimension) {

		for (int i = 0; i < numberOfCenters; i++) {
			int l;
			// fprintf(out, "%f ", centers[i].weight);
			for (l = 0; l < dimension - 1; l++) {
				if (centers[i].weight != 0.0) {
					// fprintf(out, "%f ", centers[i].coordinates[l] / centers[i].weight);
				}
				else {
					// fprintf(out, "%f ", centers[i].coordinates[l]);
				}
			}
			if (centers[i].weight != 0.0) {
				// fprintf(out, "%f", centers[i].coordinates[dimension - 1] / centers[i].weight);
			}
			else {
				// fprintf(out, "%f", centers[i].coordinates[dimension - 1]);
			}
			// fprintf(out, "\n");
		}

	};

	static int readArg(char* s, FILE* fp) {

		int c;
		int i = -1;

		while ((s[++i] = c = getc(fp)) != EOF) {
			if (c == ',' || c == '\n') {
				s[i] = '\0';
				return c;
			}
		}
		return EOF;
	};
	static int readLiadanArg(char* s, FILE* fp) {

		int c;
		int i = -1;

		while ((s[++i] = c = getc(fp)) != EOF) {
			if (c == ' ' || c == '\n') {
				s[i] = '\0';
				return c;
			}
		}
		s[i] = '\0';
		return EOF;
	};
	static int writeArg(char* s, FILE* fp, int sep) {
		fputs(s, fp);
		if (sep == ',')
			fputs(",", fp);
		if (sep == '\n')
			fputs("\n", fp);
		if (sep == ' ')
			fputs(" ", fp);
		return 1;
	};
	static int getHidHostByDomain(char* hid, char* name, char* domain, FILE* fp) {

		while (readArg(name, fp) != EOF) {
			if (strstr(name, domain) == NULL)
				strcpy(hid, name);
			else
				return 1;
		}
		return EOF;
	};
	static int getNextHost(char* t, FILE* fp) {

		int c;
		int i = -1;

		while ((t[++i] = c = getc(fp)) != EOF) {
			if (c == ' ') {                  // read hostname in t
				t[i] = '\0';
				while (getc(fp) != '\n') {    // skip label
					;
				}
				return i;
			}
		}
		return EOF;
	};
	static int getLabeledHost(char* hname, char* lb, FILE* fp) {

		int c;
		int i = -1;

		while ((hname[++i] = c = getc(fp)) != EOF) {
			if (c == ' ') {                  // read hostname in t
				hname[i] = '\0';
				break;
			}
		}
		i = -1;
		while ((lb[++i] = c = getc(fp)) != EOF) {
			if (c == ' ') {
				lb[i] = '\0';
				while (getc(fp) != '\n')     // skip rest
					;
				return i;
			}
			else if (c == '\n') {
				lb[i] = '\0';
				return i;
			}
		}
		return EOF;
	};
	static int passHeader(FILE* fpsource, FILE* fpoutput) {

		char t[1000];
		FILE* fptemp;

		while (readArg(t, fpsource) == ',') {     /* passes header to new file */
			if (fputs(t, fpoutput) == EOF) {
				// fprintf(stderr, "error in purgeUnlabeledHosts! Can't write!");
				return 0;
			}
			fputs(",", fpoutput);
		}
		fputs(t, fpoutput);
		fputs("\n", fpoutput);

		return 1;
	};

	/** debug function to print a set of points **/
	static void printPoints(int n, struct point* points) {
		int i;
		for (i = 0; i < n; i++) {
			// printf("ID %d,Dimension %d, Weight %f \n", points[i].id, points[i].dimension, points[i].weight);
		}
	};

	/** computes the target function for the given pointarray points[] (of size n) with the given array of centres centres[] (of size k) **/
	static double targetFunctionValue(int k, int n, struct point centres[], struct point points[]) {
		int i = 0;
		double sum = 0.0;
		for (i = 0; i < n; i++) {
			double nearestCost = -1.0;
			int j = 0;
			for (j = 0; j < k; j++) {
				double distance = 0.0;
				int l = 0;
				for (l = 0; l < points[i].dimension; l++) {
					//Centroid coordinate of the point
					double centroidCoordinatePoint;
					if (points[i].weight != 0.0) {
						centroidCoordinatePoint = points[i].coordinates[l] / points[i].weight;
					}
					else {
						centroidCoordinatePoint = points[i].coordinates[l];
					}
					//Centroid coordinate of the centre
					double centroidCoordinateCentre;
					if (centres[j].weight != 0.0) {
						centroidCoordinateCentre = centres[j].coordinates[l] / centres[j].weight;
					}
					else {
						centroidCoordinateCentre = centres[j].coordinates[l];
					}
					distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
						(centroidCoordinatePoint - centroidCoordinateCentre);

				}
				if (nearestCost < 0 || distance < nearestCost) {
					nearestCost = distance;
				}
			}
			sum += nearestCost * points[i].weight;
		}
		return sum;
	};

	/** computes the target function for the union of the given pointarrays setA[] (of size n_1) and setB[] (of size n_2) with the given array of centres centres[] (of size k) **/
	static double unionTargetFunctionValue(int k, int n_1, int n_2, struct point centres[], struct point setA[], struct point setB[]) {
		int n = n_1 + n_2;
		int i = 0;
		double sum = 0.0;
		for (i = 0; i < n; i++) {
			if (i < n_1) {
				double nearestCost = -1.0;
				int j = 0;
				for (j = 0; j < k; j++) {
					double distance = 0.0;
					int l = 0;
					for (l = 0; l < setA[i].dimension; l++) {
						//Centroid coordinate of the point
						double centroidCoordinatePoint;
						if (setA[i].weight != 0.0) {
							centroidCoordinatePoint = setA[i].coordinates[l] / setA[i].weight;
						}
						else {
							centroidCoordinatePoint = setA[i].coordinates[l];
						}
						//Centroid coordinate of the centre
						double centroidCoordinateCentre;
						if (centres[j].weight != 0.0) {
							centroidCoordinateCentre = centres[j].coordinates[l] / centres[j].weight;
						}
						else {
							centroidCoordinateCentre = centres[j].coordinates[l];
						}
						distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
							(centroidCoordinatePoint - centroidCoordinateCentre);
					}

					if (nearestCost < 0 || distance < nearestCost) {
						nearestCost = distance;
					}
				}
				sum += nearestCost * setA[i].weight;
			}
			else {
				double nearestCost = -1.0;
				int j = 0;
				for (j = 0; j < k; j++) {
					double distance = 0.0;
					int l = 0;
					for (l = 0; l < setB[i - n_1].dimension; l++) {
						//Centroid coordinate of the point
						double centroidCoordinatePoint;
						if (setB[i - n_1].weight != 0.0) {
							centroidCoordinatePoint = setB[i - n_1].coordinates[l] / setB[i - n_1].weight;
						}
						else {
							centroidCoordinatePoint = setB[i - n_1].coordinates[l];
						}
						//Centroid coordinate of the centre
						double centroidCoordinateCentre;
						if (centres[j].weight != 0.0) {
							centroidCoordinateCentre = centres[j].coordinates[l] / centres[j].weight;
						}
						else {
							centroidCoordinateCentre = centres[j].coordinates[l];
						}
						distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
							(centroidCoordinatePoint - centroidCoordinateCentre);
					}

					if (nearestCost < 0 || distance < nearestCost) {
						nearestCost = distance;
					}
				}
				sum += nearestCost * setB[i - n_1].weight;

			}
		}
		return sum;


	};

	/** Computes the cost of point p with the given array of centres centres[] (of size k) **/
	static double costOfPoint(int k, struct point centres[], struct point p) {
		double nearestCost = -1.0;
		int j = 0;
		for (j = 0; j < k; j++) {
			double distance = 0.0;
			int l = 0;
			for (l = 0; l < p.dimension; l++) {
				//Centroid coordinate of the point
				double centroidCoordinatePoint;
				if (p.weight != 0.0) {
					centroidCoordinatePoint = p.coordinates[l] / p.weight;
				}
				else {
					centroidCoordinatePoint = p.coordinates[l];
				}
				//Centroid coordinate of the centre
				double centroidCoordinateCentre;
				if (centres[j].weight != 0.0) {
					centroidCoordinateCentre = centres[j].coordinates[l] / centres[j].weight;
				}
				else {
					centroidCoordinateCentre = centres[j].coordinates[l];
				}
				distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
					(centroidCoordinatePoint - centroidCoordinateCentre);
			}

			if (nearestCost < 0 || distance < nearestCost) {
				nearestCost = distance;
			}
		}
		return p.weight * nearestCost;
	};

	/** Computes the index of the centre nearest to p with the given array of centres centres[] (of size k) **/
	static int determineClusterCentreKMeans(int k, struct point p, struct point centres[]) {
		int centre = 0;
		double nearestCost = -1.0;
		int j = 0;
		for (j = 0; j < k; j++) {
			double distance = 0.0;
			int l = 0;
			for (l = 0; l < p.dimension; l++) {
				//Centroid coordinate of the point
				double centroidCoordinatePoint;
				if (p.weight != 0.0) {
					centroidCoordinatePoint = p.coordinates[l] / p.weight;
				}
				else {
					centroidCoordinatePoint = p.coordinates[l];
				}
				//Centroid coordinate of the centre
				double centroidCoordinateCentre;
				if (centres[j].weight != 0.0) {
					centroidCoordinateCentre = centres[j].coordinates[l] / centres[j].weight;
				}
				else {
					centroidCoordinateCentre = centres[j].coordinates[l];
				}
				distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
					(centroidCoordinatePoint - centroidCoordinateCentre);
			}

			if (nearestCost < 0 || distance < nearestCost) {
				nearestCost = distance;
				centre = j;
			}
		}
		return centre;
	};

	/** Computes the cost of point p with centre centre **/
	static double costOfPointToCenter(struct point* p, struct point* centre) {
		if (p->weight == 0.0) {
			return 0.0;
		}

		//stores the distance between p and centre
		double distance = 0.0;

		//loop counter
		int l;

		for (l = 0; l < p->dimension; l++) {
			//Centroid coordinate of the point
			double centroidCoordinatePoint;
			if (p->weight != 0.0) {
				centroidCoordinatePoint = p->coordinates[l] / p->weight;
			}
			else {
				centroidCoordinatePoint = p->coordinates[l];
			}
			//Centroid coordinate of the centre
			double centroidCoordinateCentre;
			if (centre->weight != 0.0) {
				centroidCoordinateCentre = centre->coordinates[l] / centre->weight;
			}
			else {
				centroidCoordinateCentre = centre->coordinates[l];
			}
			distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
				(centroidCoordinatePoint - centroidCoordinateCentre);

		}
		return distance * p->weight;
	};

	static void init_genrand(unsigned long s){
		KMeans_mt[0] = s & 0xffffffffUL;
		for (KMeans_mti = 1; KMeans_mti < N; KMeans_mti++) {
			KMeans_mt[KMeans_mti] =
				(1812433253UL * (KMeans_mt[KMeans_mti - 1] ^ (KMeans_mt[KMeans_mti - 1] >> 30)) + KMeans_mti);
			/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
			/* In the previous versions, MSBs of the seed affect   */
			/* only MSBs of the array mt[].                        */
			/* 2002/01/09 modified by Makoto Matsumoto             */
			KMeans_mt[KMeans_mti] &= 0xffffffffUL;
			/* for >32 bit machines */
		}
	};
	static unsigned long genrand_int32(void) {
		unsigned long y;
		static unsigned long mag01[2] = { 0x0UL, MATRIX_A };
		/* mag01[x] = x * MATRIX_A  for x=0,1 */

		if (KMeans_mti >= N) { /* generate N words at one time */
			int kk;

			if (KMeans_mti == N + 1)   /* if init_genrand() has not been called, */
				init_genrand(5489UL); /* a default initial seed is used */

			for (kk = 0; kk < N - M; kk++) {
				y = (KMeans_mt[kk] & UPPER_MASK) | (KMeans_mt[kk + 1] & LOWER_MASK);
				KMeans_mt[kk] = KMeans_mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			for (; kk < N - 1; kk++) {
				y = (KMeans_mt[kk] & UPPER_MASK) | (KMeans_mt[kk + 1] & LOWER_MASK);
				KMeans_mt[kk] = KMeans_mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
			}
			y = (KMeans_mt[N - 1] & UPPER_MASK) | (KMeans_mt[0] & LOWER_MASK);
			KMeans_mt[N - 1] = KMeans_mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

			KMeans_mti = 0;
		}

		y = KMeans_mt[KMeans_mti++]; // mti++

		/* Tempering */
		y ^= (y >> 11);
		y ^= (y << 7) & 0x9d2c5680UL;
		y ^= (y << 15) & 0xefc60000UL;
		y ^= (y >> 18);

		return y;
	};
	static long genrand_int31(void) { return (long)(genrand_int32() >> 1); };
	static double genrand_real3(void) { return (((double)genrand_int32()) + 0.5) * (1.0 / 4294967296.0); /* divided by 2^32 */	};

	/** Randomly chooses k centres with kMeans++ distribution **/
	static struct point* chooseRandomCentres(int k, int n, int d, struct point points[]) {

		//array to store the choosen centres
		struct point* centres = (struct point*)malloc(k * sizeof(struct point));

		//choose the first centre (each point has the same probability of being choosen)
		int i = 0;


		int next = 0;
		int j = 0;
		do {//only choose from the n-i points not already choosen
			next = genrand_int31() % (n - 1);

			//check if the choosen point is not a dummy
		} while (points[next].weight < 1);

		//set j to next unchoosen point
		j = next;
		//copy the choosen point to the array
		copyPoint(&points[j], &centres[i]);

		//set the current centre for all points to the choosen centre
		for (i = 0; i < n; i++) {
			points[i].centreIndex = 0;
			points[i].curCost = costOfPointToCenter(&points[i], &centres[0]);

		}
		//choose centre 1 to k-1 with the kMeans++ distribution
		for (i = 1; i < k; i++) {

			double cost = 0.0;
			for (j = 0; j < n; j++) {
				cost += points[j].curCost;
			}

			double random = 0;
			double sum = 0.0;
			int pos = -1;

			do {

				random = genrand_real3();
				sum = 0.0;
				pos = -1;

				for (j = 0; j < n; j++) {
					sum = sum + points[j].curCost;
					if (random <= sum / cost) {
						pos = j;
						break;
					}
				}

			} while (points[pos].weight < 1);
			//copy the choosen centre
			copyPoint(&points[pos], &centres[i]);
			//check which points are closest to the new centre
			for (j = 0; j < n; j++) {
				double newCost = costOfPointToCenter(&points[j], &centres[i]);
				if (points[j].curCost > newCost) {
					points[j].curCost = newCost;
					points[j].centreIndex = i;
				}
			}

		}

		// printf("random centres: \n");
		for (i = 0; i < k; i++) {
			// printf("%d: (", i);
			int l = 0;
			for (l = 0; l < centres[i].dimension; l++) {
				// printf("%f,", centres[i].coordinates[l] / centres[i].weight);
			}
			// printf(")\n");
		}

		return centres;
	};

	/** kMeans++ algorithm for n points of dimension d with k centres **/
	static struct point* lloydPlusPlus(int k, int n, int d, struct point points[], double* resultCost) {
		// printf("starting kMeans++\n");
		//choose random centres
		struct point* centres = chooseRandomCentres(k, n, d, &points[0]);
		double cost = targetFunctionValue(k, n, &centres[0], &points[0]);
		double newCost = cost;

		std::vector<struct point> massCentres(k, struct point());
		std::vector<double> numberOfPoints(k, 0.0);

		do {
			cost = newCost;
			//reset centres of mass
			int i = 0;
			for (i = 0; i < k; i++) {

				initPoint(&massCentres[i], d);
				numberOfPoints[i] = 0.0;
			}
			//compute centres of mass
			for (i = 0; i < n; i++) {
				int centre = determineClusterCentreKMeans(k, points[i], &centres[0]);
				int l = 0;
				for (l = 0; l < massCentres[centre].dimension; l++) {
					if (points[i].weight != 0.0)
						massCentres[centre].coordinates[l] += points[i].coordinates[l];
				}
				numberOfPoints[centre] += points[i].weight;

			}

			//move centres
			for (i = 0; i < k; i++) {
				int l = 0;
				for (l = 0; l < centres[i].dimension; l++) {
					centres[i].coordinates[l] = massCentres[i].coordinates[l];
					centres[i].weight = numberOfPoints[i];
				}
			}


			//calculate costs
			newCost = targetFunctionValue(k, n, &centres[0], &points[0]);
			// printf("old cost:%f, new cost:%f \n", cost, newCost);
		} while (newCost < THRESHOLD * cost);

		// printf("Centres: \n");
		int i = 0;
		for (i = 0; i < k; i++) {
			// printf("(");
			int l = 0;
			for (l = 0; l < centres[i].dimension; l++) {
				// printf("%f,", centres[i].coordinates[l] / centres[i].weight);
			}
			// printf(")\n");
		}
		*resultCost = newCost;
		// printf("kMeans++ finished\n");
		return centres;
	};

	/** computes the hypothetical cost if the node would be split with new centers centreA, centreB **/
	static double treeNodeSplitCost(struct treeNode* node, struct point* centreA, struct point* centreB) {
		//loop counter variable
		int i, l;

		//stores the cost
		double 
			sum = 0.0, 
			distanceA,  //stores the distance between p and centreA
			distanceB,  //stores the distance between p and centreB
			centroidCoordinatePoint,  //centroid coordinate of the point
			centroidCoordinateCentre  //centroid coordinate of the centre
			;

		for (i = node->n - 1; i >= 0; --i) {
			distanceA = 0.0;
			distanceB = 0.0;
			for (l = node->points[i]->dimension - 1; l >= 0; --l) {
				if (node->points[i]->weight) centroidCoordinatePoint = node->points[i]->coordinates[l] / node->points[i]->weight;				
				else centroidCoordinatePoint = node->points[i]->coordinates[l];				
				
				if (centreA->weight) centroidCoordinateCentre = centreA->coordinates[l] / centreA->weight;				
				else centroidCoordinateCentre = centreA->coordinates[l];				
				distanceA += (centroidCoordinatePoint - centroidCoordinateCentre) * (centroidCoordinatePoint - centroidCoordinateCentre);

				if (centreB->weight) centroidCoordinateCentre = centreB->coordinates[l] / centreB->weight;
				else centroidCoordinateCentre = centreB->coordinates[l];
				distanceB += (centroidCoordinatePoint - centroidCoordinateCentre) * (centroidCoordinatePoint - centroidCoordinateCentre);
			}

			//add the cost of the closest centre to the sum
			if (distanceA < distanceB) 
				sum += distanceA * node->points[i]->weight;			
			else 
				sum += distanceB * node->points[i]->weight;		
		}

		return sum; //return the total cost
	};

	/** computes the cost of point p with the centre of treenode node **/
	static double treeNodeCostOfPoint(struct treeNode* node, struct point* p) {
		if (p->weight == 0.0) {
			return 0.0;
		}

		//stores the distance between centre and p
		double distance = 0.0;

		//loop counter variable
		int l;

		for (l = 0; l < p->dimension; l++) {
			//centroid coordinate of the point
			double centroidCoordinatePoint;
			if (p->weight != 0.0) {
				centroidCoordinatePoint = p->coordinates[l] / p->weight;
			}
			else {
				centroidCoordinatePoint = p->coordinates[l];
			}
			//centroid coordinate of the centre
			double centroidCoordinateCentre;
			if (node->centre->weight != 0.0) {
				centroidCoordinateCentre = node->centre->coordinates[l] / node->centre->weight;
			}
			else {
				centroidCoordinateCentre = node->centre->coordinates[l];
			}
			distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
				(centroidCoordinatePoint - centroidCoordinateCentre);

		}
		return distance * p->weight;
	};

	/**
	Computes the target function value of the n points of the treenode. Differs from the function "targetFunctionValue" in three things:

	1. only the centre of the treenode is used as a centre

	2. works on arrays of pointers instead on arrays of points

	3. stores the cost in the treenode
	**/
	static void treeNodeTargetFunctionValue(struct treeNode* node) {
		//loop counter variable
		int i;

		//stores the cost
		double sum = 0.0;

		for (i = 0; i < node->n; i++) {
			//stores the distance
			double distance = 0.0;

			//loop counter variable
			int l;

			for (l = 0; l < node->points[i]->dimension; l++) {
				//centroid coordinate of the point
				double centroidCoordinatePoint;
				if (node->points[i]->weight != 0.0) {
					centroidCoordinatePoint = node->points[i]->coordinates[l] / node->points[i]->weight;
				}
				else {
					centroidCoordinatePoint = node->points[i]->coordinates[l];
				}
				//centroid coordinate of the centre
				double centroidCoordinateCentre;
				if (node->centre->weight != 0.0) {
					centroidCoordinateCentre = node->centre->coordinates[l] / node->centre->weight;
				}
				else {
					centroidCoordinateCentre = node->centre->coordinates[l];
				}
				distance += (centroidCoordinatePoint - centroidCoordinateCentre) *
					(centroidCoordinatePoint - centroidCoordinateCentre);

			}

			sum += distance * node->points[i]->weight;
		}


		node->cost = sum;
	};

	/** initalizes root as a treenode with the union of setA and setB as pointset and centre as centre **/
	static void constructRoot(struct treeNode* root, struct point* setA, struct point* setB, int n_1, int n_2, struct point* centre, int centreIndex) {
		//loop counter variable
		int i;

		//the root has no parent and no child nodes in the beginning
		root->parent = NULL;
		root->lc = NULL;
		root->rc = NULL;

		//array with points to the points
		root->points = (point**)malloc(sizeof(struct point*) * (n_1 + n_2));
		root->n = n_1 + n_2;

		for (i = 0; i < root->n; i++) {
			if (i < n_1) {
				root->points[i] = &setA[i];
				root->points[i]->centreIndex = centreIndex;
			}
			else {
				root->points[i] = &setB[i - n_1];
				root->points[i]->centreIndex = centreIndex;
			}
		}

		//set the centre
		root->centre = centre;

		//calculate costs
		treeNodeTargetFunctionValue(root);
	};

	/** tests if a node is a leaf **/
	static bool isLeaf(struct treeNode* node) {

		if (node->lc == NULL && node->rc == NULL) {
			return true;
		}
		else {
			return false;
		}
	};

	/** selects a leaf node (using the kMeans++ distribution) **/
	static struct treeNode* selectNode(struct treeNode* root) {

		//random number between 0 and 1
		double random = genrand_real3();

		while (!isLeaf(root)) {
			if (root->lc->cost == 0 && root->rc->cost == 0) {
				if (root->lc->n == 0) {
					root = root->rc;
				}
				else if (root->rc->n == 0) {
					root = root->lc;
				}
				else if (random < 0.5) {
					random = genrand_real3();
					root = root->lc;
				}
				else {
					random = genrand_real3();
					root = root->rc;
				}
			}
			else {

				if (random < root->lc->cost / root->cost) {

					root = root->lc;
				}
				else {

					root = root->rc;
				}
			}
		}

		return root;
	};

	/**
	selects a new centre from the treenode (using the kMeans++ distribution)
	**/
	static struct point* chooseCentre(struct treeNode* node) {

		//How many times should we try to choose a centre ??
		int times = 3;

		//stores the nodecost if node is split with the best centre
		double minCost = node->cost;
		struct point* bestCentre = NULL;

		//loop counter variable
		int i, j;
		double sum, random, curCost;
		for (j = 0; j < times; j++) {
			//sum of the relativ cost of the points
			sum = 0.0;
			//random number between 0 and 1
			random = genrand_real3();

			for (i = 0; i < node->n; i++) {
				sum += treeNodeCostOfPoint(node, node->points[i]) / node->cost;
				if (sum >= random) {
					if (node->points[i]->weight == 0.0) {
						// printf("ERROR: CHOOSEN DUMMY NODE THOUGH OTHER AVAILABLE \n");
						return NULL;
					}
					curCost = treeNodeSplitCost(node, node->centre, node->points[i]);
					if (curCost < minCost) {
						bestCentre = node->points[i];
						minCost = curCost;
					}
					break;
				}
			}
		}
		if (bestCentre == NULL) {
			return node->points[0];
		}
		else {
			return bestCentre;
		}
	};

	/**
	returns the next centre
	**/
	static struct point* determineClosestCentre(struct point* p, struct point* centreA, struct point* centreB) {		
		int l; //loop counter variable		
		double distanceA = 0.0; //stores the distance between p and centreA
		double centroidCoordinatePoint; //centroid coordinate of the point
		double centroidCoordinateCentre; //centroid coordinate of the centre
		double distanceB = 0.0; //stores the distance between p and centreB

		for (l = p->dimension - 1; l >= 0; --l) {
			if (p->weight) centroidCoordinatePoint = p->coordinates[l] / p->weight;			
			else centroidCoordinatePoint = p->coordinates[l];	

			if (centreA->weight) centroidCoordinateCentre = centreA->coordinates[l] / centreA->weight;			
			else centroidCoordinateCentre = centreA->coordinates[l];		
			distanceA += (centroidCoordinatePoint - centroidCoordinateCentre) * (centroidCoordinatePoint - centroidCoordinateCentre);

			if (centreB->weight != 0.0) centroidCoordinateCentre = centreB->coordinates[l] / centreB->weight;
			else centroidCoordinateCentre = centreB->coordinates[l];
			distanceB += (centroidCoordinatePoint - centroidCoordinateCentre) * (centroidCoordinatePoint - centroidCoordinateCentre);
		}

		//return the nearest centre
		if (distanceA < distanceB) return centreA;		
		else return centreB;		
	};

	/**
	splits the parent node and creates two child nodes (one with the old centre and one with the new one)
	**/
	static void split(struct treeNode* parent, struct point* newCentre, int newCentreIndex) {
		//loop counter variable
		int i;

		//1. Counts how many points belong to the new and how many points belong to the old centre
		int nOld = 0;
		int nNew = 0;
		struct point* centre;
		for (i = 0; i < parent->n; i++) {
			centre = determineClosestCentre(parent->points[i], parent->centre, newCentre);
			if (centre == newCentre) nNew++;			
			else nOld++;			
		}

		//2. initalizes the arrays for the pointer

		//array for pointer on the points belonging to the old centre
		struct point** oldPoints = (point**)malloc(nOld * sizeof(struct point*));

		//array for pointer on the points belonging to the new centre
		struct point** newPoints = (point**)malloc(nNew * sizeof(struct point*));

		int indexOld = 0;
		int indexNew = 0;

		for (i = 0; i < parent->n; i++) {
			centre = determineClosestCentre(parent->points[i], parent->centre, newCentre);
			if (centre == newCentre) {
				newPoints[indexNew] = parent->points[i];
				newPoints[indexNew]->centreIndex = newCentreIndex;
				indexNew++;
			}
			else if (centre == parent->centre) {
				oldPoints[indexOld] = parent->points[i];
				indexOld++;
			}
		}

		//left child: old centre
		struct treeNode* lc = (treeNode*)malloc(sizeof(struct treeNode));
		lc->centre = parent->centre;
		lc->points = oldPoints;
		lc->n = nOld;

		lc->lc = nullptr;
		lc->rc = nullptr;
		lc->parent = parent;

		treeNodeTargetFunctionValue(lc);

		//right child: new centre
		struct treeNode* rc = (treeNode*)malloc(sizeof(struct treeNode));
		rc->centre = newCentre;
		rc->points = newPoints;
		rc->n = nNew;

		rc->lc = nullptr;
		rc->rc = nullptr;
		rc->parent = parent;

		treeNodeTargetFunctionValue(rc);

		//set childs of the parent node
		parent->lc = lc;
		parent->rc = rc;

		//propagate the cost changes to the parent nodes
		while (parent) {
			parent->cost = parent->lc->cost + parent->rc->cost;
			parent = parent->parent;
		}

	};

	/**
	Checks if the storage is completly freed
	**/
	static bool treeFinished(struct treeNode* root) {
		if (root->parent == NULL && root->lc == NULL && root->rc == NULL) {
			return true;
		}
		else {
			return false;
		}

	};

	/**
	frees a tree of its storage
	**/
	static void freeTree(struct treeNode* root) {
		while (!treeFinished(root)) {
			if (root->lc == NULL && root->rc == NULL) {
				root = root->parent;
			}
			else if (root->lc == NULL && root->rc != NULL) {
				//Schau ob rc ein Blatt ist
				if (isLeaf(root->rc)) {
					//Gebe rechtes Kind frei
					free(root->rc->points);
					free(root->rc);
					root->rc = NULL;
				}
				else {
					//Fahre mit rechtem Kind fort
					root = root->rc;
				}
			}
			else if (root->lc != NULL) {
				if (isLeaf(root->lc)) {
					free(root->lc->points);
					free(root->lc);
					root->lc = NULL;
				}
				else {
					root = root->lc;
				}
			}
		}
		free(root->points);
		free(root);
	};

	/** Constructs a coreset of size k from the union of setA and setB **/
	static void unionTreeCoreset(int k, int n_1, int n_2, int d, struct point* setA, struct point* setB, struct point* centres) {
		// printf("Computing coreset...\n");
		//total number of points
		int n = n_1 + n_2;

		//choose the first centre (each point has the same probability of being choosen)

		//stores, how many centres have been choosen yet
		int choosenPoints = 0;

		//only choose from the n-i points not already choosen
		int j = genrand_int31() % (n - choosenPoints);

		//copy the choosen point
		if (j < n_1) {
			copyPointWithoutInit(&setA[j], &centres[choosenPoints]);
		}
		else {
			j = j - n_1;
			copyPointWithoutInit(&setB[j], &centres[choosenPoints]);
		}
		struct treeNode* root = (treeNode*)malloc(sizeof(struct treeNode));
		constructRoot(root, setA, setB, n_1, n_2, &centres[choosenPoints], choosenPoints);
		choosenPoints = 1;

		//choose the remaining points
		struct treeNode* leaf;
		struct point* centre;
		while (choosenPoints < k) {
			if (root->cost > 0.0) {
				leaf = selectNode(root);
				centre = chooseCentre(leaf);
				split(leaf, centre, choosenPoints);
				copyPointWithoutInit(centre, &centres[choosenPoints]);
			}
			else {
				//create a dummy point
				copyPointWithoutInit(root->centre, &centres[choosenPoints]);
				int l;
				for (l = 0; l < root->centre->dimension; l++) {
					centres[choosenPoints].coordinates[l] = -1 * 1000000;
				}
				centres[choosenPoints].id = -1;
				centres[choosenPoints].weight = 0.0;
				centres[choosenPoints].squareSum = 0.0;

			}

			choosenPoints++;
		}

		//free the tree
		freeTree(root);

		//recalculate clustering features
		int i;
		for (i = 0; i < n; i++) {
			if (i < n_1) {
				int index = setA[i].centreIndex;
				if (centres[index].id != setA[i].id) {
					centres[index].weight += setA[i].weight;
					centres[index].squareSum += setA[i].squareSum;
					int l;
					for (l = 0; l < centres[index].dimension; l++) {
						if (setA[i].weight != 0.0) {
							centres[index].coordinates[l] += setA[i].coordinates[l];
						}
					}
				}
			}
			else {
				int index = setB[i - n_1].centreIndex;
				if (centres[index].id != setB[i - n_1].id) {
					centres[index].weight += setB[i - n_1].weight;
					centres[index].squareSum += setB[i - n_1].squareSum;
					int l;
					for (l = 0; l < centres[index].dimension; l++) {
						if (setB[i - n_1].weight != 0.0) {
							centres[index].coordinates[l] += setB[i - n_1].coordinates[l];
						}
					}
				}
			}
		}
	};

	/** initializes a bucket **/
	static void initBucket(struct Bucket* bucket, int d, int maxsize) {
		bucket->cursize = 0;
		bucket->points = (point*)malloc(maxsize * sizeof(struct point));
		bucket->spillover = (point*)malloc(maxsize * sizeof(struct point));
		int i;
		for (i = 0; i < maxsize; i++) {
			initPoint(&(bucket->points[i]), d);
			initPoint(&(bucket->spillover[i]), d);
		}
	};

	/** initializes a bucketmanager for n points with bucketsize maxsize and dimension d **/
	static void initManager(struct Bucketmanager* manager, int n, int d, int maxsize) {
		manager->numberOfBuckets = ceil(log((double)n / (double)maxsize) / log(2)) + 2;
		manager->maxBucketsize = maxsize;
		manager->buckets = (Bucket*)malloc(manager->numberOfBuckets * sizeof(struct Bucket));
		int i;
		for (i = 0; i < manager->numberOfBuckets; i++) {
			initBucket(&(manager->buckets[i]), d, maxsize);
		}
		// printf("Created manager with %d buckets of dimension %d \n", manager->numberOfBuckets, d);
	};

	/** inserts a single point into the bucketmanager **/
	static void insertPoint(struct point* p, struct Bucketmanager* manager) {

		//check if there is enough space in the first bucket
		int cursize = manager->buckets[0].cursize;
		if (cursize >= manager->maxBucketsize) {
			// printf("Bucket 0 full \n");
			//start spillover process
			int curbucket = 0;
			int nextbucket = 1;

			//check if the next bucket is empty
			if (manager->buckets[nextbucket].cursize == 0) {
				//copy the bucket	
				int i;
				for (i = 0; i < manager->maxBucketsize; i++) {

					copyPointWithoutInit(&(manager->buckets[curbucket].points[i]),
						&(manager->buckets[nextbucket].points[i]));

				}
				//bucket is now full
				manager->buckets[nextbucket].cursize = manager->maxBucketsize;
				//first bucket is now empty
				manager->buckets[curbucket].cursize = 0;
				cursize = 0;
			}
			else {
				// printf("Bucket %d full \n", nextbucket);
				//copy bucket to spillover and continue
				int i;
				for (i = 0; i < manager->maxBucketsize; i++) {

					copyPointWithoutInit(&(manager->buckets[curbucket].points[i]),
						&(manager->buckets[nextbucket].spillover[i]));

				}
				manager->buckets[0].cursize = 0;
				cursize = 0;
				curbucket++;
				nextbucket++;
				/*
				as long as the next bucket is full output the coreset to the spillover of the next bucket
				*/
				while (manager->buckets[nextbucket].cursize == manager->maxBucketsize) {
					// printf("Bucket %d full \n", nextbucket);
					unionTreeCoreset(manager->maxBucketsize, manager->maxBucketsize,
						manager->maxBucketsize, p->dimension,
						manager->buckets[curbucket].points, manager->buckets[curbucket].spillover,
						manager->buckets[nextbucket].spillover);
					//bucket now empty
					manager->buckets[curbucket].cursize = 0;
					curbucket++;
					nextbucket++;
				}
				unionTreeCoreset(manager->maxBucketsize, manager->maxBucketsize,
					manager->maxBucketsize, p->dimension,
					manager->buckets[curbucket].points, manager->buckets[curbucket].spillover,
					manager->buckets[nextbucket].points);
				manager->buckets[curbucket].cursize = 0;
				manager->buckets[nextbucket].cursize = manager->maxBucketsize;
			}

		}
		//insert point into the first bucket
		copyPointWithoutInit(p, &(manager->buckets[0].points[cursize]));
		manager->buckets[0].cursize++;
	};

	/** inserts a single point into the bucketmanager **/
	static void insertPoint(vec2d const& p, int index, struct Bucketmanager* manager) {

		//check if there is enough space in the first bucket
		int cursize = manager->buckets[0].cursize;
		if (cursize >= manager->maxBucketsize) {
			// printf("Bucket 0 full \n");
			//start spillover process
			int curbucket = 0;
			int nextbucket = 1;

			//check if the next bucket is empty
			if (manager->buckets[nextbucket].cursize == 0) {
				//copy the bucket	
				int i;
				for (i = 0; i < manager->maxBucketsize; i++) {
					copyPointWithoutInit(&(manager->buckets[curbucket].points[i]),
						&(manager->buckets[nextbucket].points[i]));
				}
				//bucket is now full
				manager->buckets[nextbucket].cursize = manager->maxBucketsize;
				//first bucket is now empty
				manager->buckets[curbucket].cursize = 0;
				cursize = 0;
			}
			else {
				// printf("Bucket %d full \n", nextbucket);
				//copy bucket to spillover and continue
				int i;
				for (i = 0; i < manager->maxBucketsize; i++) {
					copyPointWithoutInit(&(manager->buckets[curbucket].points[i]),
						&(manager->buckets[nextbucket].spillover[i]));
				}
				manager->buckets[0].cursize = 0;
				cursize = 0;
				curbucket++;
				nextbucket++;
				/*
				as long as the next bucket is full output the coreset to the spillover of the next bucket
				*/
				while (manager->buckets[nextbucket].cursize == manager->maxBucketsize) {
					// printf("Bucket %d full \n", nextbucket);
					unionTreeCoreset(manager->maxBucketsize, manager->maxBucketsize,
						manager->maxBucketsize, 2,
						manager->buckets[curbucket].points, manager->buckets[curbucket].spillover,
						manager->buckets[nextbucket].spillover);
					//bucket now empty
					manager->buckets[curbucket].cursize = 0;
					curbucket++;
					nextbucket++;
				}
				unionTreeCoreset(manager->maxBucketsize, manager->maxBucketsize,
					manager->maxBucketsize, 2,
					manager->buckets[curbucket].points, manager->buckets[curbucket].spillover,
					manager->buckets[nextbucket].points);
				manager->buckets[curbucket].cursize = 0;
				manager->buckets[nextbucket].cursize = manager->maxBucketsize;
			}

		}
		//insert point into the first bucket
		copyPointWithoutInit(p, index, &(manager->buckets[0].points[cursize]));
		manager->buckets[0].cursize++;
	};


	/**
	It may happen that the manager is not full (since n is not always a power of 2). In this case we extract the coreset
	from the manager by computing a coreset of all nonempty buckets

	Case 1: the last bucket is full
	=> n is a power of 2 and we return the contents of the last bucket

	Case2: the last bucket is not full
	=> we compute a coreset of all nonempty buckets

	this operation should only be called after the streaming process is finished
	**/
	static struct point* getCoresetFromManager(struct Bucketmanager* manager, int d) {
		struct point* coreset = nullptr;
		int i = 0;
		if (manager->buckets[manager->numberOfBuckets - 1].cursize == manager->maxBucketsize) {
			coreset = manager->buckets[manager->numberOfBuckets - 1].points;

		}
		else {
			//find the first nonempty bucket
			for (i = 0; i < manager->numberOfBuckets; i++) {
				if (manager->buckets[i].cursize != 0) {
					coreset = manager->buckets[i].points;
					break;
				}
			}
			//as long as there is a nonempty bucket compute a coreset
			int j;
			for (j = i + 1; j < manager->numberOfBuckets; j++) {
				if (manager->buckets[j].cursize != 0) {
					//output the coreset into the spillover of bucket j
					unionTreeCoreset(manager->maxBucketsize, manager->maxBucketsize,
						manager->maxBucketsize, d,
						manager->buckets[j].points, coreset,
						manager->buckets[j].spillover);
					coreset = manager->buckets[j].spillover;
				}
			}
		}
		return coreset;
	};

public:
	// Gets the approximate K-Means centers. Does not evaluate all points, and is only useful for large datasets. 
	static cweeSharedPtr<cweeList<vec2d>> GetClusters(cweeList<vec2d> const& source, int K) {
		struct point* points;
		int length;
		int dimension;
		int numberOfCentres;
		int coresetsize;
		int seed;

		length = source.Num();
		numberOfCentres = K;
		dimension = 2;
		coresetsize = std::sqrt(length);
		seed = 555;

		//compute a coreset with the streaming algorithm
		struct Bucketmanager* manager = (Bucketmanager*)malloc(sizeof(struct Bucketmanager));
		initManager(manager, length, dimension, coresetsize);

		//insert the points one by one
		int i; int l; {

			for (i = 0; i < length; i++) {					
#if 1
				insertPoint(source[i], i, manager);
#else
				struct point* p = (point*)malloc(sizeof(struct point));
				initPoint(p, dimension);
				p->weight = 1.0;

				decltype(auto) nextRow = source[i];

				p->coordinates[0] = nextRow.x;
				p->coordinates[1] = nextRow.y;
				p->squareSum = nextRow.LengthSqr();				
				p->id = i;

				insertPoint(p, manager);

				freePoint(p);
				free(p);
#endif
			}
		}
		struct point* streamingCoreset = getCoresetFromManager(manager, dimension);

		//compute 5 clusterings of the coreset with kMeans++ and take the best
		double minCost = 0.0;
		double curCost = 0.0;
		struct point* centresStreamingCoreset = lloydPlusPlus(numberOfCentres, coresetsize, dimension, streamingCoreset, &minCost);
		curCost = minCost;
		for (i = 1; i < 5; i++) {
			struct point* tmpCentresStreamingCoreset = lloydPlusPlus(numberOfCentres, coresetsize, dimension, streamingCoreset, &curCost);
			if (curCost < minCost) {
				minCost = curCost;
				centresStreamingCoreset = tmpCentresStreamingCoreset;
			}
		}
		
		auto outP = make_cwee_shared<cweeList<vec2d>>();
		auto& out = *outP;
		for (i = 0; i < numberOfCentres; ++i) {
			out.Append(vec2d(centresStreamingCoreset[i].coordinates[0] / centresStreamingCoreset[i].weight, centresStreamingCoreset[i].coordinates[1] / centresStreamingCoreset[i].weight));
		}
		return outP;
	};

};