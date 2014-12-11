#include "config.h"

#include "math/vector.h"
#include "math/matrix.h"

#include <SFML/Graphics.hpp>

#define _CRT_MATH_DEFINES
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <vector>

struct Triangle {
    inline Triangle() {}
    Vector4 vertices[3];
	sf::Color color;
};

std::vector<Triangle> read_triangles(const char *filename, sf::Color color) {
	std::vector<Triangle> ret;
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	assert(file.good());
	unsigned num_triangles = 0;
	ret.reserve(num_triangles);
	file.read((char*)&num_triangles, sizeof(num_triangles));
	for (size_t i = 0; i < num_triangles; ++i) {
		Triangle t;
		t.color = color;
		file.read((char*)&t.vertices[0], 3 * sizeof(float));
		file.read((char*)&t.vertices[1], 3 * sizeof(float));
		file.read((char*)&t.vertices[2], 3 * sizeof(float));
		assert(file.good());
		ret.push_back(t);
	}
	return ret;
}

void add_shadows(const std::vector<Triangle> &triangles, std::vector<Triangle> &triangles_shadows,
                 Vector4 light_source, sf::Color shadow_color) {
    for(const Triangle &t : triangles) {
        Triangle t_shadow;
        t_shadow.color = shadow_color;
        for(size_t i = 0; i < 3; ++i) {
            Vector4 v = t.vertices[i];
            Vector4 l_v = v - light_source;
            float t = -light_source.y/l_v.y;
            t_shadow.vertices[i] = light_source + l_v * t;
        }
        triangles_shadows.push_back(t_shadow);
    }
}

void transform_triangles(std::vector<Triangle> &triangles, const Matrix4 &m) {
    for(Triangle &t : triangles) {
        for(Vector4 &v : t.vertices) {
            v = m * v;
        }
    }
}

void perform_perspective_divide(std::vector<Triangle> &triangles) {
    for(Triangle &t : triangles) {
        for(Vector4 &v : t.vertices) {
            v.x /= v.w;
            v.y /= v.w;
            v.z /= v.w;
        }
    }
}

void perform_culling(std::vector<Triangle> &triangles) {
    for(auto it = triangles.begin(); it != triangles.end();) {
        Triangle &t = *it;
        size_t num_invisible_vertices = 0;
        for(Vector4 v : t.vertices) {
            if(v.x < -1 || v.x > 1 ||
               v.y < -1 || v.y > 1 ||
               v.z < -1 || v.z > 1)
                ++num_invisible_vertices;
        }
        Vector4 a = t.vertices[1] - t.vertices[0];
        Vector4 b = t.vertices[2] - t.vertices[1];
        if(num_invisible_vertices == 3 || a.cross(b).z < 0) { // If all are outside the area, or the backside is seen
			it = triangles.erase(it);
		} else ++it;
    }
}

static bool is_under_other_triangle(const Triangle &t1, const Triangle &t2) {
    Vector4 A = t2.vertices[0], B = t2.vertices[1], C = t2.vertices[2];
    Vector4 a = B - A, b = C - B, c = A - C;
    for(Vector4 v : t1.vertices) {
        if(a.cross(v - A).z > 0 &&
           b.cross(v - B).z > 0 &&
           c.cross(v - C).z > 0) { // Vertex is inside triangle from camera perspective
            Vector4 N = a.cross(b);
            // Ax + By + Cz - (Ax0 + By0 + Cz0) < 0
            return N.dot(v) > N.dot(A); // Is vertex behind that triangle?
        }
    }
    return t1.vertices[0].z > A.z;
}

void sort_triangles(std::vector<Triangle> &triangles)
{
    std::sort(triangles.begin(), triangles.end(), [](const Triangle &t1, const Triangle &t2) {
        if(is_under_other_triangle(t1, t2))
            return true;
        else return !is_under_other_triangle(t2, t1);
    });
}

void sfml_draw_triangles(sf::RenderTarget &target, const std::vector<Triangle> &triangles, bool outline)
{
	sf::ConvexShape convex;
	convex.setPointCount(3);
	if (outline) {
		convex.setOutlineColor(sf::Color::Yellow);
		convex.setOutlineThickness(1);
	}
    for(const Triangle &t : triangles) {
		convex.setFillColor(t.color);
		for (size_t i = 0; i < 3; ++i)
			convex.setPoint(i, { t.vertices[i].x, t.vertices[i].y });
		target.draw(convex);
    }
}

int main(int, char const**)
{
	sf::RenderWindow window(sf::VideoMode(1024, 1024), EXECUTABLE_NAME, sf::Style::Close);
    window.setView(sf::View(sf::FloatRect(-1,-1,2,2)));
    
    float alpha = (float)M_PI_2;
    const float delta = 0.1f;

	const sf::Color pg_logo_color = sf::Color(11, 51, 94);
	std::vector<Triangle> triangles = read_triangles("pg.bin", pg_logo_color);
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::Right)
                    alpha += delta;
                else if(event.key.code == sf::Keyboard::Left)
                    alpha -= delta;
            }
        }
        
        const float S = 2;
        const float N = 1;
        const float F = 128;
        const float r = 20;
        
        Vector4 eye = {r*(float)cos(alpha), -8, r*(float)sin(alpha)};
        Vector4 center = {0, -2, 0};
        Matrix4 view = make_lookat_matrix(eye, center);
        Matrix4 projection =  make_perspective_matrix(S, N, F);
        Matrix4 vp = projection * view;

        std::vector<Triangle> transformed_triangles = triangles;
        
        const sf::Color shadow_color = sf::Color(60, 60, 60);
        std::vector<Triangle> triangles_shadows;
        const Vector4 light_source = {-8, -16, 16};
        add_shadows(transformed_triangles, triangles_shadows, light_source, shadow_color);
        
        transform_triangles(transformed_triangles, vp);
        transform_triangles(triangles_shadows, vp);
        perform_perspective_divide(transformed_triangles);
        perform_perspective_divide(triangles_shadows);
        
		//perform_culling(transformed_triangles);
        perform_culling(triangles_shadows);
        
        //sort_triangles(transformed_triangles);
      
        window.clear(sf::Color(140, 140, 140));
		bool outline = false;
        sfml_draw_triangles(window, triangles_shadows, false);
		sfml_draw_triangles(window, transformed_triangles, outline);
        window.display();
        sf::sleep(sf::milliseconds(16));
    }
    
    return EXIT_SUCCESS;
}
