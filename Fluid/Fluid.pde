// https://www.openprocessing.org/sketch/455868/
final int N = 50;
final int size = (N+2)*(N+2);
final float h = 1 / N;
final float dt = 0.05;
final float source = 50000;

final float diffusion = 0.0000001;
final float viscosity = 0.00005;

float[] u = new float[size];
float[] v = new float[size];
float[] u_prev = new float[size];
float[] v_prev = new float[size];
float[] dens = new float[size];
float[] dens_prev = new float[size];

boolean isShiftPressed = false;
boolean showVelocity = false;

int blueThreshold = 0;
int redThreshold = 500;
int greenThreshold = 1000;

int IX(int i, int j) {
	return i + (N + 2) * j;
}

int PX(int x, int y) {
	return (x + width * y);
}

void settings() {
   size(1280, 900);

}

void setup() {
  println("Usage: ");
  println("Click and drag with the mouse to add fluid with velocity");
  println("Hold shift to only apply velocity");
  println("Press \'r\' to reset");
  println();
  
  background(0);
  stroke(255, 0, 0);
  for (int i = 0; i < size; i++) {
    u[i] = v[i] = u_prev[i] = v_prev[i] = dens[i] = dens_prev[i] = 0; 
  }
}

void draw() {
  background(0);
  
  //dens[IX(int(N / 2.0), int (N / 2.0))] = 0;
  //for (int i = 1; i <= N; i++) {
  //  for (int j = 1; j <= N; j++) {
  //    int index = IX(i, j);
      
  //    dens[index] -= 1;
  //  }
  //}
  
  addVelocity();
  addDensity();
  
  velStep();
  densStep();  
  
  drawDensity();
  drawVelocity();
  
  /*
  float totalDensity = 0;
  for (int i = 1; i <= N; i++) {
    for (int j = 1; j <= N; j++) {
      totalDensity += dens[IX(i, j)];  
      
      //v[IX(i, j)] += dt * 0.005 * dens[IX(i, j)];
    }
  }
  println("Total density: " + totalDensity);*/
  
  surface.setTitle(frameRate + " FPS | blueThreshold: " + blueThreshold + " | redThreshold: " + redThreshold + " | greenThreshold: " + greenThreshold);
}

void keyPressed() {
  int incrementAmount = 50;
  
  if (key == CODED && keyCode == SHIFT) {
    isShiftPressed = true; 
  } else if (key == 'r') {
    setup(); 
  } else if (key == 'v') {
    showVelocity = !showVelocity; 
  } else if (key == '1') {
    blueThreshold = max(blueThreshold - incrementAmount, 0); 
  } else if (key == '2') {
    blueThreshold += incrementAmount;
  } else if (key == '3') {
    redThreshold = max(redThreshold - incrementAmount, 0);
  } else if (key == '4') {
    redThreshold += incrementAmount; 
  } else if (key == '5') {
    greenThreshold = max(greenThreshold - incrementAmount, 0);
  } else if (key == '6') {
    greenThreshold += incrementAmount; 
  }
}

void keyReleased() {
  if (key == CODED && keyCode == SHIFT) {
    isShiftPressed = false; 
  }
}

void densStep() {
  diffuse(0, dens_prev, dens, diffusion);
  advect(0, dens, dens_prev, u, v);
}

void velStep() {
  diffuse(1, u_prev, u, viscosity);
  diffuse(2, v_prev, v, viscosity);
  
  project(u_prev, v_prev, u, v);
  
  advect(1, u, u_prev, u_prev, v_prev);
  advect(2, v, v_prev, u_prev, v_prev);
  
  project(u, v, u_prev, v_prev);
}

void project(float[] u0, float[] v0, float[] p, float[] div) {
  float h = 1.0 / float(N);

  for (int i = 1; i <= N; i++) {
    for (int j = 1; j <= N; j++) {
      div[IX(i, j)] = -0.5 * h * (u0[IX(i + 1, j)] - u0[IX(i - 1, j)] + 
                                  v0[IX(i, j + 1)] - v0[IX(i, j - 1)]);
      p[IX(i, j)] = 0;
    }
  }
  set_bnd(0, div); set_bnd(0, p);

  for (int k = 0; k < 20; k++) {
    for (int i = 1; i <= N; i++) {
      for (int j = 1; j <= N; j++) {
        p[IX(i, j)] = (div[IX(i, j)] + p[IX(i - 1, j)] + p[IX(i + 1, j)] + p[IX(i, j - 1)] + p[IX(i, j + 1)]) / 4.0;
      }
    }
    set_bnd(0, p);
  }

  for (int i = 1; i <= N; i++) {
    for (int j = 1; j <= N; j++) {
      u0[IX(i, j)] -= 0.5 * (p[IX(i + 1, j)] - p[IX(i - 1, j)]) / h;
      v0[IX(i, j)] -= 0.5 * (p[IX(i, j + 1)] - p[IX(i, j - 1)]) / h;
    }
  }
  set_bnd(1, u0);
  set_bnd(2, v0);
}

void diffuse(int b, float[] x, float[] x0, float diff) {
  float a = dt * diff * N * N;

  for (int k = 0; k < 20; k++) {
    for (int i = 1; i <= N; i++) {
      for (int j = 1; j <= N; j++) {
        x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i - 1, j)] + x[IX(i + 1, j)] + x[IX(i, j - 1)] + x[IX(i, j + 1)])) / (1 + 4 * a);
      }
    }

    set_bnd(b, x);
  }
}

void advect(int b, float[] d, float[] d0, float[] u, float[] v) {
  int i, j, i0, j0, i1, j1;
  float x, y, s0, t0, s1, t1, dt0;

  dt0 = dt * N;
  for (i = 1; i <= N; i++) {
    for (j = 1; j <= N; j++) {
      x = i - dt0 * u[IX(i, j)];
      y = j - dt0 * v[IX(i, j)];

      if (x < 0.5) x = 0.5;
      if (x > N + 0.5) x = N + 0.5;
      i0 = (int)x;
      i1 = i0 + 1;

      if (y < 0.5) y = 0.5;
      if (y > N + 0.5) y = N + 0.5;
      j0 = (int)y;
      j1 = j0 + 1;

      s1 = x - i0;
      s0 = 1 - s1;
      t1 = y - j0;
      t0 = 1 - t1;

      d[IX(i, j)] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
                    s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
    }
  }

  set_bnd(b, d);
}

void set_bnd(int b, float[] x) {
  // b==1 => reflect vertical edges
  // b==2 => reflect horizontal edges
  
  for (int i = 1; i <= N; i++) {
    // Hard edge
    x[IX(0, i)] = b == 1 ? -x[IX(1, i)] : x[IX(1, i)];
    x[IX(N + 1, i)] = b == 1 ? -x[IX(N, i)] : x[IX(N, i)];
    x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
    x[IX(i, N+1)] = b == 2 ? -x[IX(i, N)] : x[IX(i, N)];
    
    // Free edge? Fluid just disappears
    //x[IX(0, i)] = x[IX(N + 1, i)];
    //x[IX(N + 1, i)] = x[IX(0, i)];
    //x[IX(i, 0)] = x[IX(i, N + 1)];
    //x[IX(i, N+1)] = x[IX(i, 0)];
    
    // Wraparound (might multiply water though...
    //x[IX(0, i)] = x[IX(N, i)];
    //x[IX(N + 1, i)] = x[IX(1, i)];
    //x[IX(i, 0)] = x[IX(i, N)];
    //x[IX(i, N+1)] = x[IX(i, 1)];
  }
  
  // Handle the corners
  x[IX(0, 0)] = 0.5 * (x[IX(1, 0)] + x[IX(0, 1)]);
  x[IX(0, N + 1)] = 0.5 * (x[IX(1, N+1)] + x[IX(0, N)]);
  x[IX(N + 1, 0)] = 0.5 * (x[IX(N, 0)] + x[IX(N + 1, 1)]);
  x[IX(N + 1,N + 1)] = 0.5 * (x[IX(N, N + 1)] + x[IX(N + 1, N)]);
}

void addDensityToCell(int i, int j, float density) {
  if (i != 0 && j != 0 && i != N + 1 && j != N + 1) {
	  dens[IX(i, j)] += density;
  }
}

void addDensity() {
	if (mousePressed && !isShiftPressed) {
    int i = (int( (N / float(width)) * mouseX )) + 1;
    int j = (int( (N / float(height)) * mouseY )) + 1;

    addDensityToCell(i, j, dt * source);
    addDensityToCell(i + 1, j, dt * source / 2.0);
    addDensityToCell(i - 1, j, dt * source / 2.0);
    addDensityToCell(i, j + 1, dt * source / 2.0);
    addDensityToCell(i, j - 1, dt * source / 2.0);
	}
}

void addVelocity() {
  if (mousePressed) {
    int index = IX(getPixelCellIndex(mouseX, width), getPixelCellIndex(mouseY, height));
    float xv = (N / float(width)) * (mouseX - pmouseX);
    float yv = (N / float(height)) * (mouseY - pmouseY);
    u[index] += xv * (2 / (abs(xv) + 1)) * 15;
    v[index] += yv * (2 / (abs(yv) + 1)) * 15;
    
    //println("Velocity at cell " + index + ": " + u[index] + ", " + v[index]);
  }
}

void drawDensity() {
  loadPixels();
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int i = getPixelCellIndex(x, width);
      int j = getPixelCellIndex(y, height);
      float density = int(dens[IX(i, j)]);
      
      float red = 0; float green = 0; float blue = 0;
      if (density > blueThreshold) blue = density - blueThreshold;
      if (density > redThreshold) red = density - redThreshold;
      if (density > greenThreshold) green = density - greenThreshold;
      
      pixels[PX(x, y)] = color(green, red, blue, 255);
    }
  }
  updatePixels();
}

void drawVelocity() {
  if (!showVelocity) return;
  
  stroke(255, 0, 0, 255);
  
  float cellWidth = width / float(N);
  float cellHeight = height / float(N);
  float lengthFactor = 30.0;

  for (int i = 1; i <= N; i++) {
    for (int j = 1; j <= N; j++) {
      int index = IX(i, j);
      int startX = int((i - 0.5) * cellWidth);
      int startY = int((j - 0.5) * cellHeight);

      line(startX, startY, startX + u[index] * lengthFactor, startY + v[index] * lengthFactor);
      
      //println("(" + startX + ", " + startY + ") (" + (startX + u[index]) + ", " + (startY + v[index]) + ")");
    }
  }
}

int getPixelCellIndex(int pixel, int dimension) {
  return constrain(int(pixel / (dimension / float(N))) + 1, 1, N);
}
