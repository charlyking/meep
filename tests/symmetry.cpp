/* Copyright (C) 2003 Massachusetts Institute of Technology  
%
%  This program is free software; you can redistribute it and/or modify
%  it under the terms of the GNU General Public License as published by
%  the Free Software Foundation; either version 2, or (at your option)
%  any later version.
%
%  This program is distributed in the hope that it will be useful,
%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  GNU General Public License for more details.
%
%  You should have received a copy of the GNU General Public License
%  along with this program; if not, write to the Free Software Foundation,
%  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "dactyl.h"

double one(const vec &) { return 1.0; }

int compare(double a, double b, const char *n) {
  if (fabs(a-b) > fabs(b)*1.3e-14) {
    master_printf("%s differs by\t%lg out of\t%lg\n", n, a-b, b);
    master_printf("This gives a fractional error of %lg\n", fabs(a-b)/fabs(b));
    return 0;
  } else {
    return 1;
  }
}

int compare_point(const fields &f1, const fields &f2, const vec &p) {
  monitor_point m1, m_test;
  f1.get_point(&m_test, p);
  f2.get_point(&m1, p);
  for (int i=0;i<10;i++) {
    component c = (component) i;
    if (f1.v.has_field(c)) {
      complex<double> v1 = m_test.get_component(c), v2 = m1.get_component(c);
      if (abs(v1 - v2) > 0.0*2e-15*abs(v2)) {
        master_printf("%s differs:  %lg %lg out of %lg %lg\n",
               component_name(c), real(v2-v1), imag(v2-v1), real(v2), imag(v2));
        master_printf("This comes out to a fractional error of %lg\n",
               abs(v1 - v2)/abs(v2));
        master_printf("Right now I'm looking at %lg %lg, time %lg\n",
                      p.x(), p.y(), f1.time());
        f1.output_real_imaginary_slices("multi");
        f2.output_real_imaginary_slices("single");
        f1.eps_slices("multi");
        f2.eps_slices("single");
        return 0;
      }
    }
  }
  return 1;
}

int test_metal(double eps(const vec &), const char *dirname) {
  double a = 10.0;
  double ttot = 17.0;

  const volume v = voltwo(1.0, 1.0, a);
  const symmetry S = mirror(X,v);
  mat ma(v, eps, 0, S);
  mat ma1(v, eps, 0, identity());
  ma.set_output_directory(dirname);
  ma1.set_output_directory(dirname);

  fields f1(&ma1);
  f1.use_metal_everywhere();
  f1.add_point_source(Ey, 0.7, 2.5, 0.0, 4.0, vec2d(0.5,0.5));
  f1.add_point_source(Ez, 0.8, 0.6, 0.0, 4.0, vec2d(0.5,0.401));
  fields f(&ma);
  f.use_metal_everywhere();
  f.add_point_source(Ey, 0.7, 2.5, 0.0, 4.0, vec2d(0.5,0.5));
  f.add_point_source(Ez, 0.8, 0.6, 0.0, 4.0, vec2d(0.5,0.401));
  double total_energy_check_time = 8.0;
  while (f.time() < ttot) {
    f.step();
    f1.step();
    if (!compare_point(f, f1, vec2d(0.5  , 0.01))) return 0;
    if (!compare_point(f, f1, vec2d(0.5  , 0.21))) return 0;
    if (!compare_point(f, f1, vec2d(0.46 , 0.33))) return 0;
    if (!compare_point(f, f1, vec2d(0.2  , 0.2 ))) return 0;
    if (f.time() >= total_energy_check_time) {
      if (!compare(f.electric_energy_in_box(v), f1.electric_energy_in_box(v),
                   "electric energy")) return 0;
      if (!compare(f.magnetic_energy_in_box(v), f1.magnetic_energy_in_box(v),
                   "magnetic energy")) return 0;
      if (!compare(f.total_energy(), f1.total_energy(),
                   "   total energy")) return 0;
      total_energy_check_time += 5.0;
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  initialize(argc, argv);
  const char *dirname = make_output_directory(argv[0]);
  master_printf("Testing with symmetry...\n");

  if (!test_metal(one, dirname)) abort("error in test_metal vacuum\n");

  delete[] dirname;
  finished();
  exit(0);
}
