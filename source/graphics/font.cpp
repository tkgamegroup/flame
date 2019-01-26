// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/foundation.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include FT_OUTLINE_H
#include <assert.h>

// https://github.com/Chlumsky/msdfgen
namespace msdfgen
{
	// headers:

	/// Returns the smaller of the arguments.
	template <typename T>
	inline T min(T a, T b) {
		return b < a ? b : a;
	}

	/// Returns the larger of the arguments.
	template <typename T>
	inline T max(T a, T b) {
		return a < b ? b : a;
	}

	/// Returns the middle out of three values
	template <typename T>
	inline T median(T a, T b, T c) {
		return max(min(a, b), min(max(a, b), c));
	}

	/// Returns the weighted average of a and b.
	template <typename T, typename S>
	inline T mix(T a, T b, S weight) {
		return T((S(1) - weight)*a + weight * b);
	}

	/// Clamps the number to the interval from 0 to 1.
	template <typename T>
	inline T clamp(T n) {
		return n >= T(0) && n <= T(1) ? n : T(n > T(0));
	}

	/// Clamps the number to the interval from 0 to b.
	template <typename T>
	inline T clamp(T n, T b) {
		return n >= T(0) && n <= b ? n : T(n > T(0))*b;
	}

	/// Clamps the number to the interval from a to b.
	template <typename T>
	inline T clamp(T n, T a, T b) {
		return n >= a && n <= b ? n : n < a ? a : b;
	}

	/// Returns 1 for positive values, -1 for negative values, and 0 for zero.
	template <typename T>
	inline int sign(T n) {
		return (T(0) < n) - (n < T(0));
	}

	/// Returns 1 for non-negative values and -1 for negative values.
	template <typename T>
	inline int nonZeroSign(T n) {
		return 2 * (n > T(0)) - 1;
	}

	/**
	* A 2-dimensional euclidean vector with double precision.
	* Implementation based on the Vector2 template from Artery Engine.
	* @author Viktor Chlumsky
	*/
	struct Vector2 {

		double x, y;

		Vector2(double val = 0);
		Vector2(double x, double y);
		/// Sets the vector to zero.
		void reset();
		/// Sets individual elements of the vector.
		void set(double x, double y);
		/// Returns the vector's length.
		double length() const;
		/// Returns the angle of the vector in radians (atan2).
		double direction() const;
		/// Returns the normalized vector - one that has the same direction but unit length.
		Vector2 normalize(bool allowZero = false) const;
		/// Returns a vector with the same length that is orthogonal to this one.
		Vector2 getOrthogonal(bool polarity = true) const;
		/// Returns a vector with unit length that is orthogonal to this one.
		Vector2 getOrthonormal(bool polarity = true, bool allowZero = false) const;
		/// Returns a vector projected along this one.
		Vector2 project(const Vector2 &vector, bool positive = false) const;
		operator const void *() const;
		bool operator!() const;
		bool operator==(const Vector2 &other) const;
		bool operator!=(const Vector2 &other) const;
		Vector2 operator+() const;
		Vector2 operator-() const;
		Vector2 operator+(const Vector2 &other) const;
		Vector2 operator-(const Vector2 &other) const;
		Vector2 operator*(const Vector2 &other) const;
		Vector2 operator/(const Vector2 &other) const;
		Vector2 operator*(double value) const;
		Vector2 operator/(double value) const;
		Vector2 & operator+=(const Vector2 &other);
		Vector2 & operator-=(const Vector2 &other);
		Vector2 & operator*=(const Vector2 &other);
		Vector2 & operator/=(const Vector2 &other);
		Vector2 & operator*=(double value);
		Vector2 & operator/=(double value);
		/// Dot product of two vectors.
		friend double dotProduct(const Vector2 &a, const Vector2 &b);
		/// A special version of the cross product for 2D vectors (returns scalar value).
		friend double crossProduct(const Vector2 &a, const Vector2 &b);
		friend Vector2 operator*(double value, const Vector2 &vector);
		friend Vector2 operator/(double value, const Vector2 &vector);

	};

	/// A vector may also represent a point, which shall be differentiated semantically using the alias Point2.
	typedef Vector2 Point2;

	/// Represents a signed distance and alignment, which together can be compared to uniquely determine the closest edge segment.
	class SignedDistance {

	public:
		static const SignedDistance INFINITE_;

		double distance;
		double dot;

		SignedDistance();
		SignedDistance(double dist, double d);

		friend bool operator<(SignedDistance a, SignedDistance b);
		friend bool operator>(SignedDistance a, SignedDistance b);
		friend bool operator<=(SignedDistance a, SignedDistance b);
		friend bool operator>=(SignedDistance a, SignedDistance b);

	};

	/// Edge color specifies which color channels an edge belongs to.
	enum EdgeColor {
		BLACK = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3,
		BLUE = 4,
		MAGENTA = 5,
		CYAN = 6,
		WHITE = 7
	};

	// Parameters for iterative search of closest point on a cubic Bezier curve. Increase for higher precision.
#define MSDFGEN_CUBIC_SEARCH_STARTS 4
#define MSDFGEN_CUBIC_SEARCH_STEPS 4

/// An abstract edge segment.
	class EdgeSegment {

	public:
		EdgeColor color;

		EdgeSegment(EdgeColor edgeColor = WHITE) : color(edgeColor) { }
		virtual ~EdgeSegment() { }
		/// Creates a copy of the edge segment.
		virtual EdgeSegment * clone() const = 0;
		/// Returns the point on the edge specified by the parameter (between 0 and 1).
		virtual Point2 point(double param) const = 0;
		/// Returns the direction the edge has at the point specified by the parameter.
		virtual Vector2 direction(double param) const = 0;
		/// Returns the minimum signed distance between origin and the edge.
		virtual SignedDistance signedDistance(Point2 origin, double &param) const = 0;
		/// Converts a previously retrieved signed distance from origin to pseudo-distance.
		virtual void distanceToPseudoDistance(SignedDistance &distance, Point2 origin, double param) const;
		/// Adjusts the bounding box to fit the edge segment.
		virtual void bounds(double &l, double &b, double &r, double &t) const = 0;

		/// Moves the start point of the edge segment.
		virtual void moveStartPoint(Point2 to) = 0;
		/// Moves the end point of the edge segment.
		virtual void moveEndPoint(Point2 to) = 0;
		/// Splits the edge segments into thirds which together represent the original edge.
		virtual void splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const = 0;

	};

	/// A line segment.
	class LinearSegment : public EdgeSegment {

	public:
		Point2 p[2];

		LinearSegment(Point2 p0, Point2 p1, EdgeColor edgeColor = WHITE);
		LinearSegment * clone() const;
		Point2 point(double param) const;
		Vector2 direction(double param) const;
		SignedDistance signedDistance(Point2 origin, double &param) const;
		void bounds(double &l, double &b, double &r, double &t) const;

		void moveStartPoint(Point2 to);
		void moveEndPoint(Point2 to);
		void splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const;

	};

	/// A quadratic Bezier curve.
	class QuadraticSegment : public EdgeSegment {

	public:
		Point2 p[3];

		QuadraticSegment(Point2 p0, Point2 p1, Point2 p2, EdgeColor edgeColor = WHITE);
		QuadraticSegment * clone() const;
		Point2 point(double param) const;
		Vector2 direction(double param) const;
		SignedDistance signedDistance(Point2 origin, double &param) const;
		void bounds(double &l, double &b, double &r, double &t) const;

		void moveStartPoint(Point2 to);
		void moveEndPoint(Point2 to);
		void splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const;

	};

	/// A cubic Bezier curve.
	class CubicSegment : public EdgeSegment {

	public:
		Point2 p[4];

		CubicSegment(Point2 p0, Point2 p1, Point2 p2, Point2 p3, EdgeColor edgeColor = WHITE);
		CubicSegment * clone() const;
		Point2 point(double param) const;
		Vector2 direction(double param) const;
		SignedDistance signedDistance(Point2 origin, double &param) const;
		void bounds(double &l, double &b, double &r, double &t) const;

		void moveStartPoint(Point2 to);
		void moveEndPoint(Point2 to);
		void splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const;

	};

	// ax^2 + bx + c = 0
	int solveQuadratic(double x[2], double a, double b, double c);

	// ax^3 + bx^2 + cx + d = 0
	int solveCubic(double x[3], double a, double b, double c, double d);


	/// Container for a single edge of dynamic type.
	class EdgeHolder {

	public:
		EdgeHolder();
		EdgeHolder(EdgeSegment *segment);
		EdgeHolder(Point2 p0, Point2 p1, EdgeColor edgeColor = WHITE);
		EdgeHolder(Point2 p0, Point2 p1, Point2 p2, EdgeColor edgeColor = WHITE);
		EdgeHolder(Point2 p0, Point2 p1, Point2 p2, Point2 p3, EdgeColor edgeColor = WHITE);
		EdgeHolder(const EdgeHolder &orig);
#ifdef MSDFGEN_USE_CPP11
		EdgeHolder(EdgeHolder &&orig);
#endif
		~EdgeHolder();
		EdgeHolder & operator=(const EdgeHolder &orig);
#ifdef MSDFGEN_USE_CPP11
		EdgeHolder & operator=(EdgeHolder &&orig);
#endif
		EdgeSegment & operator*();
		const EdgeSegment & operator*() const;
		EdgeSegment * operator->();
		const EdgeSegment * operator->() const;
		operator EdgeSegment *();
		operator const EdgeSegment *() const;

	private:
		EdgeSegment *edgeSegment;

	};

	/// A single closed contour of a shape.
	class Contour {

	public:
		/// The sequence of edges that make up the contour.
		std::vector<EdgeHolder> edges;

		/// Adds an edge to the contour.
		void addEdge(const EdgeHolder &edge);
#ifdef MSDFGEN_USE_CPP11
		void addEdge(EdgeHolder &&edge);
#endif
		/// Creates a new edge in the contour and returns its reference.
		EdgeHolder & addEdge();
		/// Computes the bounding box of the contour.
		void bounds(double &l, double &b, double &r, double &t) const;
		/// Computes the winding of the contour. Returns 1 if positive, -1 if negative.
		int winding() const;

	};

	/// Vector shape representation.
	class Shape {

	public:
		/// The list of contours the shape consists of.
		std::vector<Contour> contours;
		/// Specifies whether the shape uses bottom-to-top (false) or top-to-bottom (true) Y coordinates.
		bool inverseYAxis;

		Shape();
		/// Adds a contour.
		void addContour(const Contour &contour);
#ifdef MSDFGEN_USE_CPP11
		void addContour(Contour &&contour);
#endif
		/// Adds a blank contour and returns its reference.
		Contour & addContour();
		/// Normalizes the shape geometry for distance field generation.
		void normalize();
		/// Performs basic checks to determine if the object represents a valid shape.
		bool validate() const;
		/// Computes the shape's bounding box.
		void bounds(double &l, double &b, double &r, double &t) const;

	};

	/// A floating-point RGB pixel.
	struct FloatRGB {
		float r, g, b;
	};

	/// A 2D image bitmap.
	template <typename T>
	class Bitmap {

	public:
		Bitmap();
		Bitmap(int width, int height);
		Bitmap(const Bitmap<T> &orig);
#ifdef MSDFGEN_USE_CPP11
		Bitmap(Bitmap<T> &&orig);
#endif
		~Bitmap();
		Bitmap<T> & operator=(const Bitmap<T> &orig);
#ifdef MSDFGEN_USE_CPP11
		Bitmap<T> & operator=(Bitmap<T> &&orig);
#endif
		/// Bitmap width in pixels.
		int width() const;
		/// Bitmap height in pixels.
		int height() const;
		T & operator()(int x, int y);
		const T & operator()(int x, int y) const;

	private:
		T *content;
		int w, h;

	};

	/** Assigns colors to edges of the shape in accordance to the multi-channel distance field technique.
	 *  May split some edges if necessary.
	 *  angleThreshold specifies the maximum angle (in radians) to be considered a corner, for example 3 (~172 degrees).
	 *  Values below 1/2 PI will be treated as the external angle.
	 */
	void edgeColoringSimple(Shape &shape, double angleThreshold, unsigned long long seed = 0);

	/// Reconstructs the shape's appearance into output from the distance field sdf.
	void renderSDF(Bitmap<float> &output, const Bitmap<float> &sdf, double pxRange = 0);
	void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<float> &sdf, double pxRange = 0);
	void renderSDF(Bitmap<float> &output, const Bitmap<FloatRGB> &sdf, double pxRange = 0);
	void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<FloatRGB> &sdf, double pxRange = 0);

	/// Snaps the values of the floating-point bitmaps into one of the 256 values representable in a standard 8-bit bitmap.
	void simulate8bit(Bitmap<float> &bitmap);
	void simulate8bit(Bitmap<FloatRGB> &bitmap);

#define MSDFGEN_VERSION "1.5"

	/// Generates a conventional single-channel signed distance field.
	void generateSDF(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate);

	/// Generates a single-channel signed pseudo-distance field.
	void generatePseudoSDF(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate);

	/// Generates a multi-channel signed distance field. Edge colors must be assigned first! (see edgeColoringSimple)
	void generateMSDF(Bitmap<FloatRGB> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, double edgeThreshold = 1.00000001);

	// Original simpler versions of the previous functions, which work well under normal circumstances, but cannot deal with overlapping contours.
	void generateSDF_legacy(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate);
	void generatePseudoSDF_legacy(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate);
	void generateMSDF_legacy(Bitmap<FloatRGB> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, double edgeThreshold = 1.00000001);

	class FreetypeHandle;
	class FontHandle;

	/// Initializes the FreeType library
	FreetypeHandle * initializeFreetype();
	/// Deinitializes the FreeType library
	void deinitializeFreetype(FreetypeHandle *library);
	/// Loads a font file and returns its handle
	FontHandle * loadFont(FreetypeHandle *library, const char *filename);
	/// Unloads a font file
	void destroyFont(FontHandle *font);
	/// Returns the size of one EM in the font's coordinate system
	bool getFontScale(double &output, FontHandle *font);
	/// Returns the width of space and tab
	bool getFontWhitespaceWidth(double &spaceAdvance, double &tabAdvance, FontHandle *font);
	/// Loads the shape prototype of a glyph from font file
	bool loadGlyph(Shape &output, FontHandle *font, int unicode, double *advance = NULL);
	/// Returns the kerning distance adjustment between two specific glyphs.
	bool getKerning(double &output, FontHandle *font, int unicode1, int unicode2);

	// source:

	const SignedDistance SignedDistance::INFINITE_(-1e240, 1);

	SignedDistance::SignedDistance() : distance(-1e240), dot(1) { }

	SignedDistance::SignedDistance(double dist, double d) : distance(dist), dot(d) { }

	bool operator<(SignedDistance a, SignedDistance b) {
		return fabs(a.distance) < fabs(b.distance) || (fabs(a.distance) == fabs(b.distance) && a.dot < b.dot);
	}

	bool operator>(SignedDistance a, SignedDistance b) {
		return fabs(a.distance) > fabs(b.distance) || (fabs(a.distance) == fabs(b.distance) && a.dot > b.dot);
	}

	bool operator<=(SignedDistance a, SignedDistance b) {
		return fabs(a.distance) < fabs(b.distance) || (fabs(a.distance) == fabs(b.distance) && a.dot <= b.dot);
	}

	bool operator>=(SignedDistance a, SignedDistance b) {
		return fabs(a.distance) > fabs(b.distance) || (fabs(a.distance) == fabs(b.distance) && a.dot >= b.dot);
	}

	Vector2::Vector2(double val) : x(val), y(val) { }

	Vector2::Vector2(double x, double y) : x(x), y(y) { }

	void Vector2::reset() {
		x = 0, y = 0;
	}

	void Vector2::set(double x, double y) {
		Vector2::x = x, Vector2::y = y;
	}

	double Vector2::length() const {
		return sqrt(x*x + y * y);
	}

	double Vector2::direction() const {
		return atan2(y, x);
	}

	Vector2 Vector2::normalize(bool allowZero) const {
		double len = length();
		if (len == 0)
			return Vector2(0, !allowZero);
		return Vector2(x / len, y / len);
	}

	Vector2 Vector2::getOrthogonal(bool polarity) const {
		return polarity ? Vector2(-y, x) : Vector2(y, -x);
	}

	Vector2 Vector2::getOrthonormal(bool polarity, bool allowZero) const {
		double len = length();
		if (len == 0)
			return polarity ? Vector2(0, !allowZero) : Vector2(0, -!allowZero);
		return polarity ? Vector2(-y / len, x / len) : Vector2(y / len, -x / len);
	}

	Vector2 Vector2::project(const Vector2 &vector, bool positive) const {
		Vector2 n = normalize(true);
		double t = dotProduct(vector, n);
		if (positive && t <= 0)
			return Vector2();
		return t * n;
	}

	Vector2::operator const void*() const {
		return x || y ? this : NULL;
	}

	bool Vector2::operator!() const {
		return !x && !y;
	}

	bool Vector2::operator==(const Vector2 &other) const {
		return x == other.x && y == other.y;
	}

	bool Vector2::operator!=(const Vector2 &other) const {
		return x != other.x || y != other.y;
	}

	Vector2 Vector2::operator+() const {
		return *this;
	}

	Vector2 Vector2::operator-() const {
		return Vector2(-x, -y);
	}

	Vector2 Vector2::operator+(const Vector2 &other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 Vector2::operator-(const Vector2 &other) const {
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 Vector2::operator*(const Vector2 &other) const {
		return Vector2(x*other.x, y*other.y);
	}

	Vector2 Vector2::operator/(const Vector2 &other) const {
		return Vector2(x / other.x, y / other.y);
	}

	Vector2 Vector2::operator*(double value) const {
		return Vector2(x*value, y*value);
	}

	Vector2 Vector2::operator/(double value) const {
		return Vector2(x / value, y / value);
	}

	Vector2 & Vector2::operator+=(const Vector2 &other) {
		x += other.x, y += other.y;
		return *this;
	}

	Vector2 & Vector2::operator-=(const Vector2 &other) {
		x -= other.x, y -= other.y;
		return *this;
	}

	Vector2 & Vector2::operator*=(const Vector2 &other) {
		x *= other.x, y *= other.y;
		return *this;
	}

	Vector2 & Vector2::operator/=(const Vector2 &other) {
		x /= other.x, y /= other.y;
		return *this;
	}

	Vector2 & Vector2::operator*=(double value) {
		x *= value, y *= value;
		return *this;
	}

	Vector2 & Vector2::operator/=(double value) {
		x /= value, y /= value;
		return *this;
	}

	double dotProduct(const Vector2 &a, const Vector2 &b) {
		return a.x*b.x + a.y*b.y;
	}

	double crossProduct(const Vector2 &a, const Vector2 &b) {
		return a.x*b.y - a.y*b.x;
	}

	Vector2 operator*(double value, const Vector2 &vector) {
		return Vector2(value*vector.x, value*vector.y);
	}

	Vector2 operator/(double value, const Vector2 &vector) {
		return Vector2(value / vector.x, value / vector.y);
	}

	int solveQuadratic(double x[2], double a, double b, double c) {
		if (fabs(a) < 1e-14) {
			if (fabs(b) < 1e-14) {
				if (c == 0)
					return -1;
				return 0;
			}
			x[0] = -c / b;
			return 1;
		}
		double dscr = b * b - 4 * a*c;
		if (dscr > 0) {
			dscr = sqrt(dscr);
			x[0] = (-b + dscr) / (2 * a);
			x[1] = (-b - dscr) / (2 * a);
			return 2;
		}
		else if (dscr == 0) {
			x[0] = -b / (2 * a);
			return 1;
		}
		else
			return 0;
	}

	int solveCubicNormed(double *x, double a, double b, double c) {
		double a2 = a * a;
		double q = (a2 - 3 * b) / 9;
		double r = (a*(2 * a2 - 9 * b) + 27 * c) / 54;
		double r2 = r * r;
		double q3 = q * q*q;
		double A, B;
		if (r2 < q3) {
			double t = r / sqrt(q3);
			if (t < -1) t = -1;
			if (t > 1) t = 1;
			t = acos(t);
			a /= 3; q = -2 * sqrt(q);
			x[0] = q * cos(t / 3) - a;
			x[1] = q * cos((t + 2 * flame::PI) / 3) - a;
			x[2] = q * cos((t - 2 * flame::PI) / 3) - a;
			return 3;
		}
		else {
			A = -pow(fabs(r) + sqrt(r2 - q3), 1 / 3.);
			if (r < 0) A = -A;
			B = A == 0 ? 0 : q / A;
			a /= 3;
			x[0] = (A + B) - a;
			x[1] = -0.5*(A + B) - a;
			x[2] = 0.5*sqrt(3.)*(A - B);
			if (fabs(x[2]) < 1e-14)
				return 2;
			return 1;
		}
	}

	int solveCubic(double x[3], double a, double b, double c, double d) {
		if (fabs(a) < 1e-14)
			return solveQuadratic(x, b, c, d);
		return solveCubicNormed(x, b / a, c / a, d / a);
	}

	void EdgeSegment::distanceToPseudoDistance(SignedDistance &distance, Point2 origin, double param) const {
		if (param < 0) {
			Vector2 dir = direction(0).normalize();
			Vector2 aq = origin - point(0);
			double ts = dotProduct(aq, dir);
			if (ts < 0) {
				double pseudoDistance = crossProduct(aq, dir);
				if (fabs(pseudoDistance) <= fabs(distance.distance)) {
					distance.distance = pseudoDistance;
					distance.dot = 0;
				}
			}
		}
		else if (param > 1) {
			Vector2 dir = direction(1).normalize();
			Vector2 bq = origin - point(1);
			double ts = dotProduct(bq, dir);
			if (ts > 0) {
				double pseudoDistance = crossProduct(bq, dir);
				if (fabs(pseudoDistance) <= fabs(distance.distance)) {
					distance.distance = pseudoDistance;
					distance.dot = 0;
				}
			}
		}
	}

	LinearSegment::LinearSegment(Point2 p0, Point2 p1, EdgeColor edgeColor) : EdgeSegment(edgeColor) {
		p[0] = p0;
		p[1] = p1;
	}

	QuadraticSegment::QuadraticSegment(Point2 p0, Point2 p1, Point2 p2, EdgeColor edgeColor) : EdgeSegment(edgeColor) {
		if (p1 == p0 || p1 == p2)
			p1 = 0.5*(p0 + p2);
		p[0] = p0;
		p[1] = p1;
		p[2] = p2;
	}

	CubicSegment::CubicSegment(Point2 p0, Point2 p1, Point2 p2, Point2 p3, EdgeColor edgeColor) : EdgeSegment(edgeColor) {
		p[0] = p0;
		p[1] = p1;
		p[2] = p2;
		p[3] = p3;
	}

	LinearSegment * LinearSegment::clone() const {
		return new LinearSegment(p[0], p[1], color);
	}

	QuadraticSegment * QuadraticSegment::clone() const {
		return new QuadraticSegment(p[0], p[1], p[2], color);
	}

	CubicSegment * CubicSegment::clone() const {
		return new CubicSegment(p[0], p[1], p[2], p[3], color);
	}

	Point2 LinearSegment::point(double param) const {
		return mix(p[0], p[1], param);
	}

	Point2 QuadraticSegment::point(double param) const {
		return mix(mix(p[0], p[1], param), mix(p[1], p[2], param), param);
	}

	Point2 CubicSegment::point(double param) const {
		Vector2 p12 = mix(p[1], p[2], param);
		return mix(mix(mix(p[0], p[1], param), p12, param), mix(p12, mix(p[2], p[3], param), param), param);
	}

	Vector2 LinearSegment::direction(double param) const {
		return p[1] - p[0];
	}

	Vector2 QuadraticSegment::direction(double param) const {
		return mix(p[1] - p[0], p[2] - p[1], param);
	}

	Vector2 CubicSegment::direction(double param) const {
		Vector2 tangent = mix(mix(p[1] - p[0], p[2] - p[1], param), mix(p[2] - p[1], p[3] - p[2], param), param);
		if (!tangent) {
			if (param == 0) return p[2] - p[0];
			if (param == 1) return p[3] - p[1];
		}
		return tangent;
	}

	SignedDistance LinearSegment::signedDistance(Point2 origin, double &param) const {
		Vector2 aq = origin - p[0];
		Vector2 ab = p[1] - p[0];
		param = dotProduct(aq, ab) / dotProduct(ab, ab);
		Vector2 eq = p[param > .5] - origin;
		double endpointDistance = eq.length();
		if (param > 0 && param < 1) {
			double orthoDistance = dotProduct(ab.getOrthonormal(false), aq);
			if (fabs(orthoDistance) < endpointDistance)
				return SignedDistance(orthoDistance, 0);
		}
		return SignedDistance(nonZeroSign(crossProduct(aq, ab))*endpointDistance, fabs(dotProduct(ab.normalize(), eq.normalize())));
	}

	SignedDistance QuadraticSegment::signedDistance(Point2 origin, double &param) const {
		Vector2 qa = p[0] - origin;
		Vector2 ab = p[1] - p[0];
		Vector2 br = p[0] + p[2] - p[1] - p[1];
		double a = dotProduct(br, br);
		double b = 3 * dotProduct(ab, br);
		double c = 2 * dotProduct(ab, ab) + dotProduct(qa, br);
		double d = dotProduct(qa, ab);
		double t[3];
		int solutions = solveCubic(t, a, b, c, d);

		double minDistance = nonZeroSign(crossProduct(ab, qa))*qa.length(); // distance from A
		param = -dotProduct(qa, ab) / dotProduct(ab, ab);
		{
			double distance = nonZeroSign(crossProduct(p[2] - p[1], p[2] - origin))*(p[2] - origin).length(); // distance from B
			if (fabs(distance) < fabs(minDistance)) {
				minDistance = distance;
				param = dotProduct(origin - p[1], p[2] - p[1]) / dotProduct(p[2] - p[1], p[2] - p[1]);
			}
		}
		for (int i = 0; i < solutions; ++i) {
			if (t[i] > 0 && t[i] < 1) {
				Point2 endpoint = p[0] + 2 * t[i] * ab + t[i] * t[i] * br;
				double distance = nonZeroSign(crossProduct(p[2] - p[0], endpoint - origin))*(endpoint - origin).length();
				if (fabs(distance) <= fabs(minDistance)) {
					minDistance = distance;
					param = t[i];
				}
			}
		}

		if (param >= 0 && param <= 1)
			return SignedDistance(minDistance, 0);
		if (param < .5)
			return SignedDistance(minDistance, fabs(dotProduct(ab.normalize(), qa.normalize())));
		else
			return SignedDistance(minDistance, fabs(dotProduct((p[2] - p[1]).normalize(), (p[2] - origin).normalize())));
	}

	SignedDistance CubicSegment::signedDistance(Point2 origin, double &param) const {
		Vector2 qa = p[0] - origin;
		Vector2 ab = p[1] - p[0];
		Vector2 br = p[2] - p[1] - ab;
		Vector2 as = (p[3] - p[2]) - (p[2] - p[1]) - br;

		Vector2 epDir = direction(0);
		double minDistance = nonZeroSign(crossProduct(epDir, qa))*qa.length(); // distance from A
		param = -dotProduct(qa, epDir) / dotProduct(epDir, epDir);
		{
			epDir = direction(1);
			double distance = nonZeroSign(crossProduct(epDir, p[3] - origin))*(p[3] - origin).length(); // distance from B
			if (fabs(distance) < fabs(minDistance)) {
				minDistance = distance;
				param = dotProduct(origin + epDir - p[3], epDir) / dotProduct(epDir, epDir);
			}
		}
		// Iterative minimum distance search
		for (int i = 0; i <= MSDFGEN_CUBIC_SEARCH_STARTS; ++i) {
			double t = (double)i / MSDFGEN_CUBIC_SEARCH_STARTS;
			for (int step = 0;; ++step) {
				Vector2 qpt = point(t) - origin;
				double distance = nonZeroSign(crossProduct(direction(t), qpt))*qpt.length();
				if (fabs(distance) < fabs(minDistance)) {
					minDistance = distance;
					param = t;
				}
				if (step == MSDFGEN_CUBIC_SEARCH_STEPS)
					break;
				// Improve t
				Vector2 d1 = 3 * as*t*t + 6 * br*t + 3 * ab;
				Vector2 d2 = 6 * as*t + 6 * br;
				t -= dotProduct(qpt, d1) / (dotProduct(d1, d1) + dotProduct(qpt, d2));
				if (t < 0 || t > 1)
					break;
			}
		}

		if (param >= 0 && param <= 1)
			return SignedDistance(minDistance, 0);
		if (param < .5)
			return SignedDistance(minDistance, fabs(dotProduct(direction(0).normalize(), qa.normalize())));
		else
			return SignedDistance(minDistance, fabs(dotProduct(direction(1).normalize(), (p[3] - origin).normalize())));
	}

	static void pointBounds(Point2 p, double &l, double &b, double &r, double &t) {
		if (p.x < l) l = p.x;
		if (p.y < b) b = p.y;
		if (p.x > r) r = p.x;
		if (p.y > t) t = p.y;
	}

	void LinearSegment::bounds(double &l, double &b, double &r, double &t) const {
		pointBounds(p[0], l, b, r, t);
		pointBounds(p[1], l, b, r, t);
	}

	void QuadraticSegment::bounds(double &l, double &b, double &r, double &t) const {
		pointBounds(p[0], l, b, r, t);
		pointBounds(p[2], l, b, r, t);
		Vector2 bot = (p[1] - p[0]) - (p[2] - p[1]);
		if (bot.x) {
			double param = (p[1].x - p[0].x) / bot.x;
			if (param > 0 && param < 1)
				pointBounds(point(param), l, b, r, t);
		}
		if (bot.y) {
			double param = (p[1].y - p[0].y) / bot.y;
			if (param > 0 && param < 1)
				pointBounds(point(param), l, b, r, t);
		}
	}

	void CubicSegment::bounds(double &l, double &b, double &r, double &t) const {
		pointBounds(p[0], l, b, r, t);
		pointBounds(p[3], l, b, r, t);
		Vector2 a0 = p[1] - p[0];
		Vector2 a1 = 2 * (p[2] - p[1] - a0);
		Vector2 a2 = p[3] - 3 * p[2] + 3 * p[1] - p[0];
		double params[2];
		int solutions;
		solutions = solveQuadratic(params, a2.x, a1.x, a0.x);
		for (int i = 0; i < solutions; ++i)
			if (params[i] > 0 && params[i] < 1)
				pointBounds(point(params[i]), l, b, r, t);
		solutions = solveQuadratic(params, a2.y, a1.y, a0.y);
		for (int i = 0; i < solutions; ++i)
			if (params[i] > 0 && params[i] < 1)
				pointBounds(point(params[i]), l, b, r, t);
	}

	void LinearSegment::moveStartPoint(Point2 to) {
		p[0] = to;
	}

	void QuadraticSegment::moveStartPoint(Point2 to) {
		Vector2 origSDir = p[0] - p[1];
		Point2 origP1 = p[1];
		p[1] += crossProduct(p[0] - p[1], to - p[0]) / crossProduct(p[0] - p[1], p[2] - p[1])*(p[2] - p[1]);
		p[0] = to;
		if (dotProduct(origSDir, p[0] - p[1]) < 0)
			p[1] = origP1;
	}

	void CubicSegment::moveStartPoint(Point2 to) {
		p[1] += to - p[0];
		p[0] = to;
	}

	void LinearSegment::moveEndPoint(Point2 to) {
		p[1] = to;
	}

	void QuadraticSegment::moveEndPoint(Point2 to) {
		Vector2 origEDir = p[2] - p[1];
		Point2 origP1 = p[1];
		p[1] += crossProduct(p[2] - p[1], to - p[2]) / crossProduct(p[2] - p[1], p[0] - p[1])*(p[0] - p[1]);
		p[2] = to;
		if (dotProduct(origEDir, p[2] - p[1]) < 0)
			p[1] = origP1;
	}

	void CubicSegment::moveEndPoint(Point2 to) {
		p[2] += to - p[3];
		p[3] = to;
	}

	void LinearSegment::splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const {
		part1 = new LinearSegment(p[0], point(1 / 3.), color);
		part2 = new LinearSegment(point(1 / 3.), point(2 / 3.), color);
		part3 = new LinearSegment(point(2 / 3.), p[1], color);
	}

	void QuadraticSegment::splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const {
		part1 = new QuadraticSegment(p[0], mix(p[0], p[1], 1 / 3.), point(1 / 3.), color);
		part2 = new QuadraticSegment(point(1 / 3.), mix(mix(p[0], p[1], 5 / 9.), mix(p[1], p[2], 4 / 9.), .5), point(2 / 3.), color);
		part3 = new QuadraticSegment(point(2 / 3.), mix(p[1], p[2], 2 / 3.), p[2], color);
	}

	void CubicSegment::splitInThirds(EdgeSegment *&part1, EdgeSegment *&part2, EdgeSegment *&part3) const {
		part1 = new CubicSegment(p[0], p[0] == p[1] ? p[0] : mix(p[0], p[1], 1 / 3.), mix(mix(p[0], p[1], 1 / 3.), mix(p[1], p[2], 1 / 3.), 1 / 3.), point(1 / 3.), color);
		part2 = new CubicSegment(point(1 / 3.),
			mix(mix(mix(p[0], p[1], 1 / 3.), mix(p[1], p[2], 1 / 3.), 1 / 3.), mix(mix(p[1], p[2], 1 / 3.), mix(p[2], p[3], 1 / 3.), 1 / 3.), 2 / 3.),
			mix(mix(mix(p[0], p[1], 2 / 3.), mix(p[1], p[2], 2 / 3.), 2 / 3.), mix(mix(p[1], p[2], 2 / 3.), mix(p[2], p[3], 2 / 3.), 2 / 3.), 1 / 3.),
			point(2 / 3.), color);
		part3 = new CubicSegment(point(2 / 3.), mix(mix(p[1], p[2], 2 / 3.), mix(p[2], p[3], 2 / 3.), 2 / 3.), p[2] == p[3] ? p[3] : mix(p[2], p[3], 2 / 3.), p[3], color);
	}

	EdgeHolder::EdgeHolder() : edgeSegment(NULL) { }

	EdgeHolder::EdgeHolder(EdgeSegment *segment) : edgeSegment(segment) { }

	EdgeHolder::EdgeHolder(Point2 p0, Point2 p1, EdgeColor edgeColor) : edgeSegment(new LinearSegment(p0, p1, edgeColor)) { }

	EdgeHolder::EdgeHolder(Point2 p0, Point2 p1, Point2 p2, EdgeColor edgeColor) : edgeSegment(new QuadraticSegment(p0, p1, p2, edgeColor)) { }

	EdgeHolder::EdgeHolder(Point2 p0, Point2 p1, Point2 p2, Point2 p3, EdgeColor edgeColor) : edgeSegment(new CubicSegment(p0, p1, p2, p3, edgeColor)) { }

	EdgeHolder::EdgeHolder(const EdgeHolder &orig) : edgeSegment(orig.edgeSegment ? orig.edgeSegment->clone() : NULL) { }

#ifdef MSDFGEN_USE_CPP11
	EdgeHolder::EdgeHolder(EdgeHolder &&orig) : edgeSegment(orig.edgeSegment) {
		orig.edgeSegment = NULL;
	}
#endif

	EdgeHolder::~EdgeHolder() {
		delete edgeSegment;
	}

	EdgeHolder & EdgeHolder::operator=(const EdgeHolder &orig) {
		delete edgeSegment;
		edgeSegment = orig.edgeSegment ? orig.edgeSegment->clone() : NULL;
		return *this;
	}

#ifdef MSDFGEN_USE_CPP11
	EdgeHolder & EdgeHolder::operator=(EdgeHolder &&orig) {
		delete edgeSegment;
		edgeSegment = orig.edgeSegment;
		orig.edgeSegment = NULL;
		return *this;
	}
#endif

	EdgeSegment & EdgeHolder::operator*() {
		return *edgeSegment;
	}

	const EdgeSegment & EdgeHolder::operator*() const {
		return *edgeSegment;
	}

	EdgeSegment * EdgeHolder::operator->() {
		return edgeSegment;
	}

	const EdgeSegment * EdgeHolder::operator->() const {
		return edgeSegment;
	}

	EdgeHolder::operator EdgeSegment *() {
		return edgeSegment;
	}

	EdgeHolder::operator const EdgeSegment *() const {
		return edgeSegment;
	}

	static double shoelace(const Point2 &a, const Point2 &b) {
		return (b.x - a.x)*(a.y + b.y);
	}

	void Contour::addEdge(const EdgeHolder &edge) {
		edges.push_back(edge);
	}

#ifdef MSDFGEN_USE_CPP11
	void Contour::addEdge(EdgeHolder &&edge) {
		edges.push_back((EdgeHolder &&)edge);
	}
#endif

	EdgeHolder & Contour::addEdge() {
		edges.resize(edges.size() + 1);
		return edges[edges.size() - 1];
	}

	void Contour::bounds(double &l, double &b, double &r, double &t) const {
		for (std::vector<EdgeHolder>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge)
			(*edge)->bounds(l, b, r, t);
	}

	int Contour::winding() const {
		if (edges.empty())
			return 0;
		double total = 0;
		if (edges.size() == 1) {
			Point2 a = edges[0]->point(0), b = edges[0]->point(1 / 3.), c = edges[0]->point(2 / 3.);
			total += shoelace(a, b);
			total += shoelace(b, c);
			total += shoelace(c, a);
		}
		else if (edges.size() == 2) {
			Point2 a = edges[0]->point(0), b = edges[0]->point(.5), c = edges[1]->point(0), d = edges[1]->point(.5);
			total += shoelace(a, b);
			total += shoelace(b, c);
			total += shoelace(c, d);
			total += shoelace(d, a);
		}
		else {
			Point2 prev = edges[edges.size() - 1]->point(0);
			for (std::vector<EdgeHolder>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge) {
				Point2 cur = (*edge)->point(0);
				total += shoelace(prev, cur);
				prev = cur;
			}
		}
		return sign(total);
	}

	Shape::Shape() : inverseYAxis(false) { }

	void Shape::addContour(const Contour &contour) {
		contours.push_back(contour);
	}

#ifdef MSDFGEN_USE_CPP11
	void Shape::addContour(Contour &&contour) {
		contours.push_back((Contour &&)contour);
	}
#endif

	Contour & Shape::addContour() {
		contours.resize(contours.size() + 1);
		return contours[contours.size() - 1];
	}

	bool Shape::validate() const {
		for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour) {
			if (!contour->edges.empty()) {
				Point2 corner = (*(contour->edges.end() - 1))->point(1);
				for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
					if (!*edge)
						return false;
					if ((*edge)->point(0) != corner)
						return false;
					corner = (*edge)->point(1);
				}
			}
		}
		return true;
	}

	void Shape::normalize() {
		for (std::vector<Contour>::iterator contour = contours.begin(); contour != contours.end(); ++contour)
			if (contour->edges.size() == 1) {
				EdgeSegment *parts[3] = { };
				contour->edges[0]->splitInThirds(parts[0], parts[1], parts[2]);
				contour->edges.clear();
				contour->edges.push_back(EdgeHolder(parts[0]));
				contour->edges.push_back(EdgeHolder(parts[1]));
				contour->edges.push_back(EdgeHolder(parts[2]));
			}
	}

	void Shape::bounds(double &l, double &b, double &r, double &t) const {
		for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour)
			contour->bounds(l, b, r, t);
	}

	template <typename T>
	Bitmap<T>::Bitmap() : content(NULL), w(0), h(0) { }

	template <typename T>
	Bitmap<T>::Bitmap(int width, int height) : w(width), h(height) {
		content = new T[w*h];
	}

	template <typename T>
	Bitmap<T>::Bitmap(const Bitmap<T> &orig) : w(orig.w), h(orig.h) {
		content = new T[w*h];
		memcpy(content, orig.content, w*h * sizeof(T));
	}

#ifdef MSDFGEN_USE_CPP11
	template <typename T>
	Bitmap<T>::Bitmap(Bitmap<T> &&orig) : content(orig.content), w(orig.w), h(orig.h) {
		orig.content = NULL;
	}
#endif

	template <typename T>
	Bitmap<T>::~Bitmap() {
		delete[] content;
	}

	template <typename T>
	Bitmap<T> & Bitmap<T>::operator=(const Bitmap<T> &orig) {
		delete[] content;
		w = orig.w, h = orig.h;
		content = new T[w*h];
		memcpy(content, orig.content, w*h * sizeof(T));
		return *this;
	}

#ifdef MSDFGEN_USE_CPP11
	template <typename T>
	Bitmap<T> & Bitmap<T>::operator=(Bitmap<T> &&orig) {
		delete[] content;
		content = orig.content;
		w = orig.w, h = orig.h;
		orig.content = NULL;
		return *this;
	}
#endif

	template <typename T>
	int Bitmap<T>::width() const {
		return w;
	}

	template <typename T>
	int Bitmap<T>::height() const {
		return h;
	}

	template <typename T>
	T & Bitmap<T>::operator()(int x, int y) {
		return content[y*w + x];
	}

	template <typename T>
	const T & Bitmap<T>::operator()(int x, int y) const {
		return content[y*w + x];
	}

	template class Bitmap<float>;
	template class Bitmap<FloatRGB>;


	static bool isCorner(const Vector2 &aDir, const Vector2 &bDir, double crossThreshold) {
		return dotProduct(aDir, bDir) <= 0 || fabs(crossProduct(aDir, bDir)) > crossThreshold;
	}

	static void switchColor(EdgeColor &color, unsigned long long &seed, EdgeColor banned = BLACK) {
		EdgeColor combined = EdgeColor(color&banned);
		if (combined == RED || combined == GREEN || combined == BLUE) {
			color = EdgeColor(combined^WHITE);
			return;
		}
		if (color == BLACK || color == WHITE) {
			static const EdgeColor start[3] = { CYAN, MAGENTA, YELLOW };
			color = start[seed % 3];
			seed /= 3;
			return;
		}
		int shifted = color << (1 + (seed & 1));
		color = EdgeColor((shifted | shifted >> 3)&WHITE);
		seed >>= 1;
	}

	void edgeColoringSimple(Shape &shape, double angleThreshold, unsigned long long seed) {
		double crossThreshold = sin(angleThreshold);
		std::vector<int> corners;
		for (std::vector<Contour>::iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour) {
			// Identify corners
			corners.clear();
			if (!contour->edges.empty()) {
				Vector2 prevDirection = (*(contour->edges.end() - 1))->direction(1);
				int index = 0;
				for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge, ++index) {
					if (isCorner(prevDirection.normalize(), (*edge)->direction(0).normalize(), crossThreshold))
						corners.push_back(index);
					prevDirection = (*edge)->direction(1);
				}
			}

			// Smooth contour
			if (corners.empty())
				for (std::vector<EdgeHolder>::iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge)
					(*edge)->color = WHITE;
			// "Teardrop" case
			else if (corners.size() == 1) {
				EdgeColor colors[3] = { WHITE, WHITE };
				switchColor(colors[0], seed);
				switchColor(colors[2] = colors[0], seed);
				int corner = corners[0];
				if (contour->edges.size() >= 3) {
					int m = contour->edges.size();
					for (int i = 0; i < m; ++i)
						contour->edges[(corner + i) % m]->color = (colors + 1)[int(3 + 2.875*i / (m - 1) - 1.4375 + .5) - 3];
				}
				else if (contour->edges.size() >= 1) {
					// Less than three edge segments for three colors => edges must be split
					EdgeSegment *parts[7] = { };
					contour->edges[0]->splitInThirds(parts[0 + 3 * corner], parts[1 + 3 * corner], parts[2 + 3 * corner]);
					if (contour->edges.size() >= 2) {
						contour->edges[1]->splitInThirds(parts[3 - 3 * corner], parts[4 - 3 * corner], parts[5 - 3 * corner]);
						parts[0]->color = parts[1]->color = colors[0];
						parts[2]->color = parts[3]->color = colors[1];
						parts[4]->color = parts[5]->color = colors[2];
					}
					else {
						parts[0]->color = colors[0];
						parts[1]->color = colors[1];
						parts[2]->color = colors[2];
					}
					contour->edges.clear();
					for (int i = 0; parts[i]; ++i)
						contour->edges.push_back(EdgeHolder(parts[i]));
				}
			}
			// Multiple corners
			else {
				int cornerCount = corners.size();
				int spline = 0;
				int start = corners[0];
				int m = contour->edges.size();
				EdgeColor color = WHITE;
				switchColor(color, seed);
				EdgeColor initialColor = color;
				for (int i = 0; i < m; ++i) {
					int index = (start + i) % m;
					if (spline + 1 < cornerCount && corners[spline + 1] == index) {
						++spline;
						switchColor(color, seed, EdgeColor((spline == cornerCount - 1)*initialColor));
					}
					contour->edges[index]->color = color;
				}
			}
		}
	}

	template <typename S>
	inline FloatRGB mix(FloatRGB a, FloatRGB b, S weight) {
		FloatRGB output = {
			mix(a.r, b.r, weight),
			mix(a.g, b.g, weight),
			mix(a.b, b.b, weight)
		};
		return output;
	}

	template <typename T>
	static T sample(const Bitmap<T> &bitmap, Point2 pos) {
		int w = bitmap.width(), h = bitmap.height();
		double x = pos.x*w - .5;
		double y = pos.y*h - .5;
		int l = (int)floor(x);
		int b = (int)floor(y);
		int r = l + 1;
		int t = b + 1;
		double lr = x - l;
		double bt = y - b;
		l = clamp(l, w - 1), r = clamp(r, w - 1);
		b = clamp(b, h - 1), t = clamp(t, h - 1);
		return mix(mix(bitmap(l, b), bitmap(r, b), lr), mix(bitmap(l, t), bitmap(r, t), lr), bt);
	}

	static float distVal(float dist, double pxRange) {
		if (!pxRange)
			return dist > .5;
		return (float)clamp((dist - .5)*pxRange + .5);
	}

	void renderSDF(Bitmap<float> &output, const Bitmap<float> &sdf, double pxRange) {
		int w = output.width(), h = output.height();
		pxRange *= (double)(w + h) / (sdf.width() + sdf.height());
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				float s = sample(sdf, Point2((x + .5) / w, (y + .5) / h));
				output(x, y) = distVal(s, pxRange);
			}
	}

	void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<float> &sdf, double pxRange) {
		int w = output.width(), h = output.height();
		pxRange *= (double)(w + h) / (sdf.width() + sdf.height());
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				float s = sample(sdf, Point2((x + .5) / w, (y + .5) / h));
				float v = distVal(s, pxRange);
				output(x, y).r = v;
				output(x, y).g = v;
				output(x, y).b = v;
			}
	}

	void renderSDF(Bitmap<float> &output, const Bitmap<FloatRGB> &sdf, double pxRange) {
		int w = output.width(), h = output.height();
		pxRange *= (double)(w + h) / (sdf.width() + sdf.height());
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				FloatRGB s = sample(sdf, Point2((x + .5) / w, (y + .5) / h));
				output(x, y) = distVal(median(s.r, s.g, s.b), pxRange);
			}
	}

	void renderSDF(Bitmap<FloatRGB> &output, const Bitmap<FloatRGB> &sdf, double pxRange) {
		int w = output.width(), h = output.height();
		pxRange *= (double)(w + h) / (sdf.width() + sdf.height());
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				FloatRGB s = sample(sdf, Point2((x + .5) / w, (y + .5) / h));
				output(x, y).r = distVal(s.r, pxRange);
				output(x, y).g = distVal(s.g, pxRange);
				output(x, y).b = distVal(s.b, pxRange);
			}
	}

	void simulate8bit(Bitmap<float> &bitmap) {
		int w = bitmap.width(), h = bitmap.height();
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				unsigned char v = clamp(int(bitmap(x, y) * 0x100), 0xff);
				bitmap(x, y) = v / 255.f;
			}
	}

	void simulate8bit(Bitmap<FloatRGB> &bitmap) {
		int w = bitmap.width(), h = bitmap.height();
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				unsigned char r = clamp(int(bitmap(x, y).r * 0x100), 0xff);
				unsigned char g = clamp(int(bitmap(x, y).g * 0x100), 0xff);
				unsigned char b = clamp(int(bitmap(x, y).b * 0x100), 0xff);
				bitmap(x, y).r = r / 255.f;
				bitmap(x, y).g = g / 255.f;
				bitmap(x, y).b = b / 255.f;
			}
	}

	struct MultiDistance {
		double r, g, b;
		double med;
	};

	static inline bool pixelClash(const FloatRGB &a, const FloatRGB &b, double threshold) {
		// Only consider pair where both are on the inside or both are on the outside
		bool aIn = (a.r > .5f) + (a.g > .5f) + (a.b > .5f) >= 2;
		bool bIn = (b.r > .5f) + (b.g > .5f) + (b.b > .5f) >= 2;
		if (aIn != bIn) return false;
		// If the change is 0 <-> 1 or 2 <-> 3 channels and not 1 <-> 1 or 2 <-> 2, it is not a clash
		if ((a.r > .5f && a.g > .5f && a.b > .5f) || (a.r < .5f && a.g < .5f && a.b < .5f)
			|| (b.r > .5f && b.g > .5f && b.b > .5f) || (b.r < .5f && b.g < .5f && b.b < .5f))
			return false;
		// Find which color is which: _a, _b = the changing channels, _c = the remaining one
		float aa, ab, ba, bb, ac, bc;
		if ((a.r > .5f) != (b.r > .5f) && (a.r < .5f) != (b.r < .5f)) {
			aa = a.r, ba = b.r;
			if ((a.g > .5f) != (b.g > .5f) && (a.g < .5f) != (b.g < .5f)) {
				ab = a.g, bb = b.g;
				ac = a.b, bc = b.b;
			}
			else if ((a.b > .5f) != (b.b > .5f) && (a.b < .5f) != (b.b < .5f)) {
				ab = a.b, bb = b.b;
				ac = a.g, bc = b.g;
			}
			else
				return false; // this should never happen
		}
		else if ((a.g > .5f) != (b.g > .5f) && (a.g < .5f) != (b.g < .5f)
			&& (a.b > .5f) != (b.b > .5f) && (a.b < .5f) != (b.b < .5f)) {
			aa = a.g, ba = b.g;
			ab = a.b, bb = b.b;
			ac = a.r, bc = b.r;
		}
		else
			return false;
		// Find if the channels are in fact discontinuous
		return (fabsf(aa - ba) >= threshold)
			&& (fabsf(ab - bb) >= threshold)
			&& fabsf(ac - .5f) >= fabsf(bc - .5f); // Out of the pair, only flag the pixel farther from a shape edge
	}

	void msdfErrorCorrection(Bitmap<FloatRGB> &output, const Vector2 &threshold) {
		std::vector<std::pair<int, int> > clashes;
		int w = output.width(), h = output.height();
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x) {
				if ((x > 0 && pixelClash(output(x, y), output(x - 1, y), threshold.x))
					|| (x < w - 1 && pixelClash(output(x, y), output(x + 1, y), threshold.x))
					|| (y > 0 && pixelClash(output(x, y), output(x, y - 1), threshold.y))
					|| (y < h - 1 && pixelClash(output(x, y), output(x, y + 1), threshold.y)))
					clashes.push_back(std::make_pair(x, y));
			}
		for (std::vector<std::pair<int, int> >::const_iterator clash = clashes.begin(); clash != clashes.end(); ++clash) {
			FloatRGB &pixel = output(clash->first, clash->second);
			float med = median(pixel.r, pixel.g, pixel.b);
			pixel.r = med, pixel.g = med, pixel.b = med;
		}
	}

	void generateSDF(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate) {
		int contourCount = shape.contours.size();
		int w = output.width(), h = output.height();
		std::vector<int> windings;
		windings.reserve(contourCount);
		for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
			windings.push_back(contour->winding());

#ifdef MSDFGEN_USE_OPENMP
#pragma omp parallel
#endif
		{
			std::vector<double> contourSD;
			contourSD.resize(contourCount);
#ifdef MSDFGEN_USE_OPENMP
#pragma omp for
#endif
			for (int y = 0; y < h; ++y) {
				int row = shape.inverseYAxis ? h - y - 1 : y;
				for (int x = 0; x < w; ++x) {
					double dummy;
					Point2 p = Vector2(x + .5, y + .5) / scale - translate;
					double negDist = -SignedDistance::INFINITE_.distance;
					double posDist = SignedDistance::INFINITE_.distance;
					int winding = 0;

					std::vector<Contour>::const_iterator contour = shape.contours.begin();
					for (int i = 0; i < contourCount; ++i, ++contour) {
						SignedDistance minDistance;
						for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
							SignedDistance distance = (*edge)->signedDistance(p, dummy);
							if (distance < minDistance)
								minDistance = distance;
						}
						contourSD[i] = minDistance.distance;
						if (windings[i] > 0 && minDistance.distance >= 0 && fabs(minDistance.distance) < fabs(posDist))
							posDist = minDistance.distance;
						if (windings[i] < 0 && minDistance.distance <= 0 && fabs(minDistance.distance) < fabs(negDist))
							negDist = minDistance.distance;
					}

					double sd = SignedDistance::INFINITE_.distance;
					if (posDist >= 0 && fabs(posDist) <= fabs(negDist)) {
						sd = posDist;
						winding = 1;
						for (int i = 0; i < contourCount; ++i)
							if (windings[i] > 0 && contourSD[i] > sd && fabs(contourSD[i]) < fabs(negDist))
								sd = contourSD[i];
					}
					else if (negDist <= 0 && fabs(negDist) <= fabs(posDist)) {
						sd = negDist;
						winding = -1;
						for (int i = 0; i < contourCount; ++i)
							if (windings[i] < 0 && contourSD[i] < sd && fabs(contourSD[i]) < fabs(posDist))
								sd = contourSD[i];
					}
					for (int i = 0; i < contourCount; ++i)
						if (windings[i] != winding && fabs(contourSD[i]) < fabs(sd))
							sd = contourSD[i];

					output(x, row) = float(sd / range + .5);
				}
			}
		}
	}

	void generatePseudoSDF(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate) {
		int contourCount = shape.contours.size();
		int w = output.width(), h = output.height();
		std::vector<int> windings;
		windings.reserve(contourCount);
		for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
			windings.push_back(contour->winding());

#ifdef MSDFGEN_USE_OPENMP
#pragma omp parallel
#endif
		{
			std::vector<double> contourSD;
			contourSD.resize(contourCount);
#ifdef MSDFGEN_USE_OPENMP
#pragma omp for
#endif
			for (int y = 0; y < h; ++y) {
				int row = shape.inverseYAxis ? h - y - 1 : y;
				for (int x = 0; x < w; ++x) {
					Point2 p = Vector2(x + .5, y + .5) / scale - translate;
					double sd = SignedDistance::INFINITE_.distance;
					double negDist = -SignedDistance::INFINITE_.distance;
					double posDist = SignedDistance::INFINITE_.distance;
					int winding = 0;

					std::vector<Contour>::const_iterator contour = shape.contours.begin();
					for (int i = 0; i < contourCount; ++i, ++contour) {
						SignedDistance minDistance;
						const EdgeHolder *nearEdge = NULL;
						double nearParam = 0;
						for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
							double param;
							SignedDistance distance = (*edge)->signedDistance(p, param);
							if (distance < minDistance) {
								minDistance = distance;
								nearEdge = &*edge;
								nearParam = param;
							}
						}
						if (fabs(minDistance.distance) < fabs(sd)) {
							sd = minDistance.distance;
							winding = -windings[i];
						}
						if (nearEdge)
							(*nearEdge)->distanceToPseudoDistance(minDistance, p, nearParam);
						contourSD[i] = minDistance.distance;
						if (windings[i] > 0 && minDistance.distance >= 0 && fabs(minDistance.distance) < fabs(posDist))
							posDist = minDistance.distance;
						if (windings[i] < 0 && minDistance.distance <= 0 && fabs(minDistance.distance) < fabs(negDist))
							negDist = minDistance.distance;
					}

					double psd = SignedDistance::INFINITE_.distance;
					if (posDist >= 0 && fabs(posDist) <= fabs(negDist)) {
						psd = posDist;
						winding = 1;
						for (int i = 0; i < contourCount; ++i)
							if (windings[i] > 0 && contourSD[i] > psd && fabs(contourSD[i]) < fabs(negDist))
								psd = contourSD[i];
					}
					else if (negDist <= 0 && fabs(negDist) <= fabs(posDist)) {
						psd = negDist;
						winding = -1;
						for (int i = 0; i < contourCount; ++i)
							if (windings[i] < 0 && contourSD[i] < psd && fabs(contourSD[i]) < fabs(posDist))
								psd = contourSD[i];
					}
					for (int i = 0; i < contourCount; ++i)
						if (windings[i] != winding && fabs(contourSD[i]) < fabs(psd))
							psd = contourSD[i];

					output(x, row) = float(psd / range + .5);
				}
			}
		}
	}

	void generateMSDF(Bitmap<FloatRGB> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, double edgeThreshold) {
		int contourCount = shape.contours.size();
		int w = output.width(), h = output.height();
		std::vector<int> windings;
		windings.reserve(contourCount);
		for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
			windings.push_back(contour->winding());

#ifdef MSDFGEN_USE_OPENMP
#pragma omp parallel
#endif
		{
			std::vector<MultiDistance> contourSD;
			contourSD.resize(contourCount);
#ifdef MSDFGEN_USE_OPENMP
#pragma omp for
#endif
			for (int y = 0; y < h; ++y) {
				int row = shape.inverseYAxis ? h - y - 1 : y;
				for (int x = 0; x < w; ++x) {
					Point2 p = Vector2(x + .5, y + .5) / scale - translate;

					struct EdgePoint {
						SignedDistance minDistance;
						const EdgeHolder *nearEdge;
						double nearParam;
					} sr, sg, sb;
					sr.nearEdge = sg.nearEdge = sb.nearEdge = NULL;
					sr.nearParam = sg.nearParam = sb.nearParam = 0;
					double d = fabs(SignedDistance::INFINITE_.distance);
					double negDist = -SignedDistance::INFINITE_.distance;
					double posDist = SignedDistance::INFINITE_.distance;
					int winding = 0;

					std::vector<Contour>::const_iterator contour = shape.contours.begin();
					for (int i = 0; i < contourCount; ++i, ++contour) {
						EdgePoint r, g, b;
						r.nearEdge = g.nearEdge = b.nearEdge = NULL;
						r.nearParam = g.nearParam = b.nearParam = 0;

						for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
							double param;
							SignedDistance distance = (*edge)->signedDistance(p, param);
							if ((*edge)->color&RED && distance < r.minDistance) {
								r.minDistance = distance;
								r.nearEdge = &*edge;
								r.nearParam = param;
							}
							if ((*edge)->color&GREEN && distance < g.minDistance) {
								g.minDistance = distance;
								g.nearEdge = &*edge;
								g.nearParam = param;
							}
							if ((*edge)->color&BLUE && distance < b.minDistance) {
								b.minDistance = distance;
								b.nearEdge = &*edge;
								b.nearParam = param;
							}
						}
						if (r.minDistance < sr.minDistance)
							sr = r;
						if (g.minDistance < sg.minDistance)
							sg = g;
						if (b.minDistance < sb.minDistance)
							sb = b;

						double medMinDistance = fabs(median(r.minDistance.distance, g.minDistance.distance, b.minDistance.distance));
						if (medMinDistance < d) {
							d = medMinDistance;
							winding = -windings[i];
						}
						if (r.nearEdge)
							(*r.nearEdge)->distanceToPseudoDistance(r.minDistance, p, r.nearParam);
						if (g.nearEdge)
							(*g.nearEdge)->distanceToPseudoDistance(g.minDistance, p, g.nearParam);
						if (b.nearEdge)
							(*b.nearEdge)->distanceToPseudoDistance(b.minDistance, p, b.nearParam);
						medMinDistance = median(r.minDistance.distance, g.minDistance.distance, b.minDistance.distance);
						contourSD[i].r = r.minDistance.distance;
						contourSD[i].g = g.minDistance.distance;
						contourSD[i].b = b.minDistance.distance;
						contourSD[i].med = medMinDistance;
						if (windings[i] > 0 && medMinDistance >= 0 && fabs(medMinDistance) < fabs(posDist))
							posDist = medMinDistance;
						if (windings[i] < 0 && medMinDistance <= 0 && fabs(medMinDistance) < fabs(negDist))
							negDist = medMinDistance;
					}
					if (sr.nearEdge)
						(*sr.nearEdge)->distanceToPseudoDistance(sr.minDistance, p, sr.nearParam);
					if (sg.nearEdge)
						(*sg.nearEdge)->distanceToPseudoDistance(sg.minDistance, p, sg.nearParam);
					if (sb.nearEdge)
						(*sb.nearEdge)->distanceToPseudoDistance(sb.minDistance, p, sb.nearParam);

					MultiDistance msd;
					msd.r = msd.g = msd.b = msd.med = SignedDistance::INFINITE_.distance;
					if (posDist >= 0 && fabs(posDist) <= fabs(negDist)) {
						msd.med = SignedDistance::INFINITE_.distance;
						winding = 1;
						for (int i = 0; i < contourCount; ++i)
							if (windings[i] > 0 && contourSD[i].med > msd.med && fabs(contourSD[i].med) < fabs(negDist))
								msd = contourSD[i];
					}
					else if (negDist <= 0 && fabs(negDist) <= fabs(posDist)) {
						msd.med = -SignedDistance::INFINITE_.distance;
						winding = -1;
						for (int i = 0; i < contourCount; ++i)
							if (windings[i] < 0 && contourSD[i].med < msd.med && fabs(contourSD[i].med) < fabs(posDist))
								msd = contourSD[i];
					}
					for (int i = 0; i < contourCount; ++i)
						if (windings[i] != winding && fabs(contourSD[i].med) < fabs(msd.med))
							msd = contourSD[i];
					if (median(sr.minDistance.distance, sg.minDistance.distance, sb.minDistance.distance) == msd.med) {
						msd.r = sr.minDistance.distance;
						msd.g = sg.minDistance.distance;
						msd.b = sb.minDistance.distance;
					}

					output(x, row).r = float(msd.r / range + .5);
					output(x, row).g = float(msd.g / range + .5);
					output(x, row).b = float(msd.b / range + .5);
				}
			}
		}

		if (edgeThreshold > 0)
			msdfErrorCorrection(output, edgeThreshold / (scale*range));
	}

	void generateSDF_legacy(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate) {
		int w = output.width(), h = output.height();
#ifdef MSDFGEN_USE_OPENMP
#pragma omp parallel for
#endif
		for (int y = 0; y < h; ++y) {
			int row = shape.inverseYAxis ? h - y - 1 : y;
			for (int x = 0; x < w; ++x) {
				double dummy;
				Point2 p = Vector2(x + .5, y + .5) / scale - translate;
				SignedDistance minDistance;
				for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
					for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
						SignedDistance distance = (*edge)->signedDistance(p, dummy);
						if (distance < minDistance)
							minDistance = distance;
					}
				output(x, row) = float(minDistance.distance / range + .5);
			}
		}
	}

	void generatePseudoSDF_legacy(Bitmap<float> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate) {
		int w = output.width(), h = output.height();
#ifdef MSDFGEN_USE_OPENMP
#pragma omp parallel for
#endif
		for (int y = 0; y < h; ++y) {
			int row = shape.inverseYAxis ? h - y - 1 : y;
			for (int x = 0; x < w; ++x) {
				Point2 p = Vector2(x + .5, y + .5) / scale - translate;
				SignedDistance minDistance;
				const EdgeHolder *nearEdge = NULL;
				double nearParam = 0;
				for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
					for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
						double param;
						SignedDistance distance = (*edge)->signedDistance(p, param);
						if (distance < minDistance) {
							minDistance = distance;
							nearEdge = &*edge;
							nearParam = param;
						}
					}
				if (nearEdge)
					(*nearEdge)->distanceToPseudoDistance(minDistance, p, nearParam);
				output(x, row) = float(minDistance.distance / range + .5);
			}
		}
	}

	void generateMSDF_legacy(Bitmap<FloatRGB> &output, const Shape &shape, double range, const Vector2 &scale, const Vector2 &translate, double edgeThreshold) {
		int w = output.width(), h = output.height();
#ifdef MSDFGEN_USE_OPENMP
#pragma omp parallel for
#endif
		for (int y = 0; y < h; ++y) {
			int row = shape.inverseYAxis ? h - y - 1 : y;
			for (int x = 0; x < w; ++x) {
				Point2 p = Vector2(x + .5, y + .5) / scale - translate;

				struct {
					SignedDistance minDistance;
					const EdgeHolder *nearEdge;
					double nearParam;
				} r, g, b;
				r.nearEdge = g.nearEdge = b.nearEdge = NULL;
				r.nearParam = g.nearParam = b.nearParam = 0;

				for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
					for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
						double param;
						SignedDistance distance = (*edge)->signedDistance(p, param);
						if ((*edge)->color&RED && distance < r.minDistance) {
							r.minDistance = distance;
							r.nearEdge = &*edge;
							r.nearParam = param;
						}
						if ((*edge)->color&GREEN && distance < g.minDistance) {
							g.minDistance = distance;
							g.nearEdge = &*edge;
							g.nearParam = param;
						}
						if ((*edge)->color&BLUE && distance < b.minDistance) {
							b.minDistance = distance;
							b.nearEdge = &*edge;
							b.nearParam = param;
						}
					}

				if (r.nearEdge)
					(*r.nearEdge)->distanceToPseudoDistance(r.minDistance, p, r.nearParam);
				if (g.nearEdge)
					(*g.nearEdge)->distanceToPseudoDistance(g.minDistance, p, g.nearParam);
				if (b.nearEdge)
					(*b.nearEdge)->distanceToPseudoDistance(b.minDistance, p, b.nearParam);
				output(x, row).r = float(r.minDistance.distance / range + .5);
				output(x, row).g = float(g.minDistance.distance / range + .5);
				output(x, row).b = float(b.minDistance.distance / range + .5);
			}
		}

		if (edgeThreshold > 0)
			msdfErrorCorrection(output, edgeThreshold / (scale*range));
	}

#define REQUIRE(cond) { if (!(cond)) return false; }

	class FreetypeHandle {
		friend FreetypeHandle * initializeFreetype();
		friend void deinitializeFreetype(FreetypeHandle *library);
		friend FontHandle * loadFont(FreetypeHandle *library, const char *filename);

		FT_Library library;

	};

	class FontHandle {
		friend FontHandle * loadFont(FreetypeHandle *library, const char *filename);
		friend void destroyFont(FontHandle *font);
		friend bool getFontScale(double &output, FontHandle *font);
		friend bool getFontWhitespaceWidth(double &spaceAdvance, double &tabAdvance, FontHandle *font);
		friend bool loadGlyph(Shape &output, FontHandle *font, int unicode, double *advance);
		friend bool getKerning(double &output, FontHandle *font, int unicode1, int unicode2);

		FT_Face face;

	};

	struct FtContext {
		Point2 position;
		Shape *shape;
		Contour *contour;
	};

	static Point2 ftPoint2(const FT_Vector &vector) {
		return Point2(vector.x / 64., vector.y / 64.);
	}

	static int ftMoveTo(const FT_Vector *to, void *user) {
		FtContext *context = reinterpret_cast<FtContext *>(user);
		context->contour = &context->shape->addContour();
		context->position = ftPoint2(*to);
		return 0;
	}

	static int ftLineTo(const FT_Vector *to, void *user) {
		FtContext *context = reinterpret_cast<FtContext *>(user);
		context->contour->addEdge(new LinearSegment(context->position, ftPoint2(*to)));
		context->position = ftPoint2(*to);
		return 0;
	}

	static int ftConicTo(const FT_Vector *control, const FT_Vector *to, void *user) {
		FtContext *context = reinterpret_cast<FtContext *>(user);
		context->contour->addEdge(new QuadraticSegment(context->position, ftPoint2(*control), ftPoint2(*to)));
		context->position = ftPoint2(*to);
		return 0;
	}

	static int ftCubicTo(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user) {
		FtContext *context = reinterpret_cast<FtContext *>(user);
		context->contour->addEdge(new CubicSegment(context->position, ftPoint2(*control1), ftPoint2(*control2), ftPoint2(*to)));
		context->position = ftPoint2(*to);
		return 0;
	}

	FreetypeHandle * initializeFreetype() {
		FreetypeHandle *handle = new FreetypeHandle;
		FT_Error error = FT_Init_FreeType(&handle->library);
		if (error) {
			delete handle;
			return NULL;
		}
		return handle;
	}

	void deinitializeFreetype(FreetypeHandle *library) {
		FT_Done_FreeType(library->library);
		delete library;
	}

	FontHandle * loadFont(FreetypeHandle *library, const char *filename) {
		if (!library)
			return NULL;
		FontHandle *handle = new FontHandle;
		FT_Error error = FT_New_Face(library->library, filename, 0, &handle->face);
		if (error) {
			delete handle;
			return NULL;
		}
		return handle;
	}

	void destroyFont(FontHandle *font) {
		FT_Done_Face(font->face);
		delete font;
	}

	bool getFontScale(double &output, FontHandle *font) {
		output = font->face->units_per_EM / 64.;
		return true;
	}

	bool getFontWhitespaceWidth(double &spaceAdvance, double &tabAdvance, FontHandle *font) {
		FT_Error error = FT_Load_Char(font->face, ' ', FT_LOAD_NO_SCALE);
		if (error)
			return false;
		spaceAdvance = font->face->glyph->advance.x / 64.;
		error = FT_Load_Char(font->face, '\t', FT_LOAD_NO_SCALE);
		if (error)
			return false;
		tabAdvance = font->face->glyph->advance.x / 64.;
		return true;
	}

	bool loadGlyph(Shape &output, FontHandle *font, int unicode, double *advance) {
		if (!font)
			return false;
		FT_Error error = FT_Load_Char(font->face, unicode, FT_LOAD_RENDER);
		if (error)
			return false;
		output.contours.clear();
		output.inverseYAxis = false;
		if (advance)
			*advance = font->face->glyph->advance.x / 64.;

		FtContext context = { };
		context.shape = &output;
		FT_Outline_Funcs ftFunctions;
		ftFunctions.move_to = &ftMoveTo;
		ftFunctions.line_to = &ftLineTo;
		ftFunctions.conic_to = &ftConicTo;
		ftFunctions.cubic_to = &ftCubicTo;
		ftFunctions.shift = 0;
		ftFunctions.delta = 0;
		error = FT_Outline_Decompose(&font->face->glyph->outline, &ftFunctions, &context);
		if (error)
			return false;
		return true;
	}

	bool getKerning(double &output, FontHandle *font, int unicode1, int unicode2) {
		FT_Vector kerning;
		if (FT_Get_Kerning(font->face, FT_Get_Char_Index(font->face, unicode1), FT_Get_Char_Index(font->face, unicode2), FT_KERNING_UNSCALED, &kerning)) {
			output = 0;
			return false;
		}
		output = kerning.x / 64.;
		return true;
	}
}

namespace flame
{
	namespace graphics
	{
		FT_Library ft_library = 0;

		void get_latin_code_range(wchar_t &out_begin, wchar_t &out_end)
		{
			out_begin = 0x20;
			out_end = 0xff;
		}

		struct FontPrivate : Font
		{
			Device *d;
			bool sdf;

			std::pair<std::unique_ptr<char[]>, int> font_file;
			FT_Face ft_face;
			int pixel_height;
			int ascender;
			int max_width;

			Glyph *map[65536];
			Glyph *glyph_head;

			Image *atlas;

			inline FontPrivate(Device *_d, const wchar_t *filename, int _pixel_height, bool _sdf) :
				d(_d),
				pixel_height(_pixel_height),
				sdf(_sdf)
			{
				memset(map, 0, sizeof(map));

				if (!ft_library)
				{
					FT_Init_FreeType(&ft_library);
					FT_Library_SetLcdFilter(ft_library, FT_LCD_FILTER_DEFAULT);
				}

				font_file = get_file_content(filename);
				FT_New_Memory_Face(ft_library, (unsigned char*)font_file.first.get(), font_file.second, 0, &ft_face);
				FT_Size_RequestRec ft_req = {};
				ft_req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
				ft_req.height = pixel_height * 64;
				FT_Request_Size(ft_face, &ft_req);
				ascender = ft_face->size->metrics.ascender / 64;
				max_width = ft_face->size->metrics.max_advance;

				atlas = Image::create(d, Format_R8G8B8A8_UNORM, Ivec2(512), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst, MemPropDevice);
				atlas->set_pixel(511, 511, Bvec4(255));
			}

			inline ~FontPrivate()
			{
				FT_Done_Face(ft_face);
				Image::destroy(atlas);
			}

			inline const Glyph &get_glyph(wchar_t unicode)
			{
				if (!map[unicode])
				{
					auto g = new Glyph(unicode);

					FT_Load_Char(ft_face, unicode, FT_LOAD_TARGET_LCD);
					auto ft_glyph = ft_face->glyph;
					g->size = Vec2(ft_glyph->bitmap.width / 3, ft_glyph->bitmap.rows);
					g->off = Vec2(ft_glyph->bitmap_left, ascender + g->size.y - ft_glyph->metrics.horiBearingY / 64.f);
					g->advance = ft_glyph->advance.x / 64;
					g->ascent = ascender;

					map[unicode] = g;
					glyphs.emplace_back(g);

					auto curr_ft_face_idx = 0;
					for (auto &g : glyphs)
					{
						auto ft_face = ft_faces[curr_ft_face_idx].face;
						auto ft_g = ft_face->glyph;

						FT_Load_Char(ft_face, g.unicode, FT_LOAD_TARGET_LCD);
						FT_Render_Glyph(ft_g, FT_RENDER_MODE_LCD);

						auto pitch1 = ft_g->bitmap.pitch;
						auto pitch2 = image->pitch;
						auto width = ft_g->bitmap.width / 3;
						for (auto y = 0; y < ft_g->bitmap.rows; y++)
						{
							for (auto x = 0; x < width; x++)
							{
								Bvec4 col;
								col.x = ft_g->bitmap.buffer[y * pitch1 + x * 3 + 0];
								col.y = ft_g->bitmap.buffer[y * pitch1 + x * 3 + 1];
								col.z = ft_g->bitmap.buffer[y * pitch1 + x * 3 + 2];
								col.w = 255;

								image->data[(g.img_off.y + y) * pitch2 + ((g.img_off.x + x) * 4 + 0)] =
									col.x;
								image->data[(g.img_off.y + y) * pitch2 + ((g.img_off.x + x) * 4 + 1)] =
									col.y;
								image->data[(g.img_off.y + y) * pitch2 + ((g.img_off.x + x) * 4 + 2)] =
									col.z;
								image->data[(g.img_off.y + y) * pitch2 + ((g.img_off.x + x) * 4 + 3)] =
									col.w;
							}
						}

						g.uv0 = Vec2(g.img_off.x, g.img_off.y + g.size.y) / image->size;
						g.uv1 = Vec2(g.img_off.x + g.size.x, g.img_off.y) / image->size;
					}

					if (sdf_scale > 0.f)
					{
						const float range = 4.f;
						auto img_height = 0;
						auto pos = Ivec2(0);
						auto line_height = 0;

						for (auto &g : glyphs)
						{
							if (g.sdf_img_off.x == -1)
								continue;

							auto size = g.size * sdf_scale;
							size += range * 2.f;

							line_height = max(line_height, size.y);
							if (pos.x + size.x > img_width)
							{
								pos.x = 0;
								pos.y += line_height;
								line_height = 0;
							}
							img_height = max(img_height, pos.y + size.y);

							g.sdf_img_off = pos;

							pos.x += size.x;
						}

						for (auto &g : glyphs)
						{
							if (g.sdf_img_off.x != -1)
							{
								auto ft_face = ft_faces[curr_ft_face_idx].face;
								auto ft_g = ft_face->glyph;
								void *ptr = ft_face;

								msdfgen::Shape shape;
								msdfgen::loadGlyph(shape, (msdfgen::FontHandle*)&ptr, g.unicode);

								auto size = g.size * sdf_scale;
								size += range * 2.f;

								shape.normalize();
								msdfgen::edgeColoringSimple(shape, 3.f);
								msdfgen::Bitmap<msdfgen::FloatRGB> bmp(size.x, size.y);
								msdfgen::generateMSDF(bmp, shape, range, sdf_scale, msdfgen::Vector2(-g.off.x, g.off.y - g.ascent) + range / sdf_scale);

								auto pitch = image->pitch;
								for (auto y = 0; y < size.y; y++)
								{
									for (auto x = 0; x < size.x; x++)
									{
										image->data[(g.sdf_img_off.y + y) * pitch + (g.sdf_img_off.x + x) * 4 + 0] =
											clamp(bmp(x, y).r * 255.f, 0.f, 255.f);
										image->data[(g.sdf_img_off.y + y) * pitch + (g.sdf_img_off.x + x) * 4 + 1] =
											clamp(bmp(x, y).g * 255.f, 0.f, 255.f);
										image->data[(g.sdf_img_off.y + y) * pitch + (g.sdf_img_off.x + x) * 4 + 2] =
											clamp(bmp(x, y).b * 255.f, 0.f, 255.f);
										image->data[(g.sdf_img_off.y + y) * pitch + (g.sdf_img_off.x + x) * 4 + 3] =
											255.f;
									}
								}

								g.uv0_sdf = (Vec2(g.sdf_img_off) + range) / image->size;
								g.uv1_sdf = (Vec2(g.sdf_img_off + size) - range) / image->size;
							}
						}
					}
				}

				return *map[unicode];
			}

			inline int get_text_width(const wchar_t *text_beg, const wchar_t *text_end)
			{
				auto w = 0;
				auto s = text_beg;
				if (text_end == nullptr)
				{
					while (*s)
					{
						auto g = get_glyph(*s);
						w += g.advance;
						s++;
					}
				}
				else
				{
					while (s != text_end)
					{
						auto g = get_glyph(*s);
						w += g.advance;
						s++;
					}
				}
				return w;
			}
		};

		const Glyph &Font::get_glyph(wchar_t unicode)
		{
			return ((FontPrivate*)this)->get_glyph(unicode);
		}

		int Font::get_text_width(const wchar_t *text_beg, const wchar_t *text_end)
		{
			return ((FontPrivate*)this)->get_text_width(text_beg, text_end);
		}

		Image *Font::get_atlas() const
		{
			return ((FontPrivate*)this)->atlas;
		}

		Font *Font::create(Device *d, const wchar_t *filename, int pixel_height, bool sdf)
		{
			return new FontPrivate(d, filename, pixel_height, sdf);
		}

		void Font::destroy(Font *f)
		{
			delete (FontPrivate*)f;
		}

	}
}

