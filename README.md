# Advanced Ray Tracing and Path Tracing Renderer

This project is an implementation of advanced rendering techniques using Ray Tracing and Path Tracing. It incorporates multiple features such as BRDF sampling, light sampling, BVH optimization, and gamma correction to produce high-quality images.

## Features

- **Binary Tracing**: Basic rendering with binary shading.
- **Phong Shading**: Implements Blinn-Phong shading for realistic lighting.
- **Normal Rendering**: Includes normal visualization for debugging.
- **Path Tracing**: Adds global illumination and realistic light transport.
- **BRDF Sampling**: Enhances Path Tracing with material-based sampling.
- **BVH Optimization**: Accelerates rendering using Bounding Volume Hierarchy.
- **Threading Support**: Multithreading for faster image generation.
- **Gamma Correction**: Linear and Reinhard tone mapping options.

## Requirements

- C++17 or higher
- A JSON library (e.g., `nlohmann/json`)
- A compiler such as GCC, Clang, or MSVC
- Make (for building)

## How to Build and Run

1. **Clone the Repository**
   ```bash
   git clone <your-repo-url>
   cd <repository-name>
   make run
   ```
2.   **RUN the project**
  ./mainEmmi <json-path> <output-image1> <output-image2>

3.   **Select Rendering Mode**
  Press 1 for binary Trace, 2 for Bling Phong effect, 3 for Normal, 4 for path Tracing, 5 for path Tracing with BRDF sampling.

4. **Output**
  The program generates two output images:<br/>
  	•	scene_linear.ppm: Gamma-corrected image.<br/>
  	•	scene_normal.ppm: Normal image without gamma correction.<br/>
