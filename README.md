# RaytracingWeekend
Implemented a basic raytracer by following the [_Ray Tracing in One Weekend_](https://raytracing.github.io/books/RayTracingInOneWeekend.html) books by Peter Shirley. 

## Improvements
My version extends the basic tracer described in Book 1 by adding a simple dispersion model, GUI, and multithreading support as well as more primitives.

### Performance
1. Multithreading support
2. Changes to the pseudo random generation. This is still WIP, but calculating a large set of random numbers in advance improved performance by more than 2x
3. replacing shared_ptr with raw pointers for the hit_records. As these are being created at every ray bounce, the overhead of incrementing and decrementing the internal reference counter became quite significant. 10% speedup.
4. Minor restructuring to take advantage of compiler optimization and reduced branching 
5. Faster cube intersection method adapted from the [PSRaytracing repository](https://github.com/define-private-public/PSRayTracing)

## Impressions
![prism cube demo](Image_Outputs/precomputed_random512535.png)
![dispersion demo](Image_Outputs/emissive_dispersive.png)
![emission demo](Image_Outputs/only_emissive.png)
