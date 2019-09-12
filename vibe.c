#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define width 320
#define height 256
#define color_space_max 255
#define total_frames 1286
#define sample_count 20
#define cardinality_threshold_min 2
#define color_space_radius 20
#define neighborhood_size 4
#define use_sample_chance 16

FILE *file_read;
FILE *file_write;

uint8_t *pixels;
uint8_t *last_pixels;
uint8_t *seg_map;
uint8_t **samples;

struct pixel {
  int r;
  int g;
  int b;
};

int color_space_distance( struct pixel pixel_one, struct pixel pixel_two ) {
  int r_diff = abs( pixel_two.r - pixel_one.r );
  int g_diff = abs( pixel_two.g - pixel_one.g );
  int b_diff = abs( pixel_two.b - pixel_one.b );

  return ( int ) sqrt( ( double ) ( r_diff * r_diff + g_diff * g_diff + b_diff * b_diff ) );
}

int get_random_neighbor_index( int index, bool immediate ) {
  int horizontal_random;
  int vertical_random;
  int search_size;

  if ( immediate ) {
    search_size = 2;
  } else {
    search_size = neighborhood_size;
  }

  int x = index % width;
  int y = index / width;

  do {
    horizontal_random = rand() % ( search_size + 1 ) - ( search_size / 2 );
  } while ( x + horizontal_random < 0 || x + horizontal_random >= width || horizontal_random == 0 );

  do {
    vertical_random = rand() % ( search_size + 1 ) - ( search_size / 2 );
  } while ( y + vertical_random < 0 || y + vertical_random >= height || vertical_random == 0 );

  return index + vertical_random * width + horizontal_random;
}

bool in_background( struct pixel pixel_value, int index ) {
  int in_radius_count = 0;

  for ( int i = 0; i < sample_count; i++ ) {
    struct pixel sample = { samples[index][i * 3], samples[index][i * 3 + 1], samples[index][i * 3 + 2] };

    if ( color_space_distance( pixel_value, sample ) < color_space_radius ) {
      in_radius_count += 1;
    }

    if ( in_radius_count >= cardinality_threshold_min ) {
      return true;
    }
  }

  return false;
}

void initialize_model() {
  for ( int i = 0; i < width * height; i++ ) {
    last_pixels[i * 3] = pixels[i * 3];
    last_pixels[i * 3 + 1] = pixels[i * 3 + 1];
    last_pixels[i * 3 + 2] = pixels[i * 3 + 2];

    for ( int j = 0; j < sample_count; j++ ) {
      int sample_index = get_random_neighbor_index( i, false );

      samples[i][j * 3] = pixels[sample_index * 3];
      samples[i][j * 3 + 1] = pixels[sample_index * 3 + 1];
      samples[i][j * 3 + 2] = pixels[sample_index * 3 + 2];
    }
  }
}

void update_own_pixel_sample( int index ) {
  int random_sample = rand() % 20;

  samples[index][random_sample * 3] = pixels[index * 3];
  samples[index][random_sample * 3 + 1] = pixels[index * 3 + 1];
  samples[index][random_sample * 3 + 2] = pixels[index * 3 + 2];
}

void update_neighbor_pixel_sample( int index, bool immediate ) {
  int random_sample = rand() % 20;
  int neighbor_index = get_random_neighbor_index( index, immediate );

  samples[neighbor_index][random_sample * 3] = pixels[index * 3];
  samples[neighbor_index][random_sample * 3 + 1] = pixels[index * 3 + 1];
  samples[neighbor_index][random_sample * 3 + 2] = pixels[index * 3 + 2];
}

void load_image( char* file_name ) {
  char *line;
  size_t len = 0;
  size_t nread = 0;
  int pixel_count = 0;

  bool found_type = false;
  bool found_width = false;
  bool found_height = false;
  bool found_color_max = false;

  file_read = fopen( file_name, "r" );

  if ( file_read == NULL ) {
    printf( "Cannot open file\n" );
    exit( 0 );
  }

  while ( ( nread = getline( &line, &len, file_read ) ) != -1 ) {
    char item[5] = { 0, 0, 0, 0, 0 };
    int item_count = 0;

    int r = -1;
    int g = -1;
    int b = -1;

    for ( int i = 0; i < nread; i++ ) {
      if ( line[i] != ' ' && line[i] != '\n' ) {
        item[item_count] = line[i];
        item_count += 1;
      }

      if ( ( line[i] == ' ' || i == nread - 1 ) && item_count > 0 ) {
        if ( !found_type ) {
          if ( item_count != 2 || item[0] != 'P' || item[1] != '3' ) {
            printf( "Invalid ppm type\n" );
            exit( 0 );
          } else {
            found_type = true;
          }
        } else if ( !found_width ) {
          int temp_width = atoi( item );

          if ( temp_width != width ) {
            printf( "Invalid width\n" );
            exit( 0 );
          } else {
            found_width = true;
          }
        } else if ( !found_height ) {
          int temp_height = atoi( item );

          if ( temp_height != height ) {
            printf( "Invalid height\n" );
            exit( 0 );
          } else {
            found_height = true;
          }
        } else if ( !found_color_max ) {
          int temp_color_space_max = atoi( item );

          if ( temp_color_space_max != color_space_max ) {
            printf( "Invalid color space max\n" );
            exit( 0 );
          } else {
            found_color_max = true;
          }
        } else {
          if ( r == -1 ) {
            r = atoi( item );

            pixels[pixel_count * 3] = ( uint8_t )r;
          } else if ( g == -1 ) {
            g = atoi( item );

            pixels[pixel_count * 3 + 1] = ( uint8_t )g;
          } else {
            b = atoi( item );

            pixels[pixel_count * 3 + 2] = ( uint8_t )b;
            pixel_count += 1;

            r = -1;
            g = -1;
            b = -1;
          }
        }

        item[0] = 0;
        item[1] = 0;
        item[2] = 0;
        item[3] = 0;
        item[4] = 0;

        item_count = 0;
      }
    }
  }

  fclose( file_read );
}

void save_seg_map( char* file_name ) {
  file_write = fopen( file_name, "w" );

  fprintf( file_write, "P3\n%d %d %d\n", width, height, color_space_max );

  for ( int i = 0; i < width * height; i++ ) {
    int color = 0;

    if ( seg_map[i] == 1 ) {
      color = color_space_max;
    }

    fprintf( file_write, "%d %d %d\n", color, color, color );
  }

  fclose( file_write );
}

int draw_seg_map_pixel( int i ) {
  struct pixel current_pixel = { pixels[i * 3], pixels[i * 3 + 1], pixels[i * 3 + 2] };
  struct pixel last_pixel = { last_pixels[i * 3], last_pixels[i * 3 + 1], last_pixels[i * 3 + 2] };

  last_pixels[i * 3] = pixels[i * 3];
  last_pixels[i * 3 + 1] = pixels[i * 3 + 1];
  last_pixels[i * 3 + 2] = pixels[i * 3 + 2];

  if ( in_background( current_pixel, i ) ) {
    seg_map[i] = 0;

    int ran = rand() % use_sample_chance;

    if ( ran == 0 ) {
      update_own_pixel_sample( i );
    }

    ran = rand() % use_sample_chance;

    if ( ran == 0 ) {
      update_neighbor_pixel_sample( i, false );
    } else {
      ran = rand() % use_sample_chance;

      if ( ran == 0 ) {
        update_neighbor_pixel_sample( i, true );
      }
    }

    return 0;
  } else {
    seg_map[i] = 1;

    int ran = rand() % use_sample_chance;

    // in foreground
    bool up = false;
    bool right = false;
    bool down = false;
    bool left = false;

    if ( i - width >= 0 ) {
      up = seg_map[i - width] == 1;
    }

    if ( ( i % width ) + 1 < width ) {
      right = seg_map[i + 1] == 1;
    }

    if ( i + width < width * height ) {
      down = seg_map[i + width] == 1;
    }

    if ( ( i % width ) - 1 > 0 ) {
      left = seg_map[i - 1];
    }

    if ( ran == 0 && color_space_distance( current_pixel, last_pixel ) < color_space_radius && !( up && right && down && left ) ) {
      update_own_pixel_sample( i );
    }

    return 1;
  }
}

void draw_seg_map() {
  int foreground = 0;

  for ( int i = 0; i < width * height; i++ ) {
    foreground += draw_seg_map_pixel( i );
  }

  if ( ( ( float ) foreground ) / ( ( float ) width * height ) > 0.5 ) {
    initialize_model();
  }
}

void vibe_main( int frame_num ) {
  char in_file_name[50];
  char out_file_name[50];

  sprintf( in_file_name, "input%d.ppm", frame_num );
  sprintf( out_file_name, "output/output%05d.ppm", frame_num );

  char *line;
  size_t len = 0;
  size_t nread = 0;
  int pixel_count = 0;

  bool found_type = false;
  bool found_width = false;
  bool found_height = false;
  bool found_color_max = false;

  file_read = fopen( in_file_name, "r" );

  file_write = fopen( out_file_name, "w" );

  fprintf( file_write, "P3\n%d %d %d\n", width, height, color_space_max );

  if ( file_read == NULL ) {
    printf( "Cannot open file\n" );
    exit( 0 );
  }

  while ( ( nread = getline( &line, &len, file_read ) ) != -1 ) {
    char item[5] = { 0, 0, 0, 0, 0 };
    int item_count = 0;

    int r = -1;
    int g = -1;
    int b = -1;

    for ( int i = 0; i < nread; i++ ) {
      if ( line[i] != ' ' && line[i] != '\n' ) {
        item[item_count] = line[i];
        item_count += 1;
      }

      if ( ( line[i] == ' ' || i == nread - 1 ) && item_count > 0 ) {
        if ( !found_type ) {
          if ( item_count != 2 || item[0] != 'P' || item[1] != '3' ) {
            printf( "Invalid ppm type\n" );
            exit( 0 );
          } else {
            found_type = true;
          }
        } else if ( !found_width ) {
          int temp_width = atoi( item );

          if ( temp_width != width ) {
            printf( "Invalid width\n" );
            exit( 0 );
          } else {
            found_width = true;
          }
        } else if ( !found_height ) {
          int temp_height = atoi( item );

          if ( temp_height != height ) {
            printf( "Invalid height\n" );
            exit( 0 );
          } else {
            found_height = true;
          }
        } else if ( !found_color_max ) {
          int temp_color_space_max = atoi( item );

          if ( temp_color_space_max != color_space_max ) {
            printf( "Invalid color space max\n" );
            exit( 0 );
          } else {
            found_color_max = true;
          }
        } else {
          if ( r == -1 ) {
            r = atoi( item );

            pixels[pixel_count * 3] = ( uint8_t )r;
          } else if ( g == -1 ) {
            g = atoi( item );

            pixels[pixel_count * 3 + 1] = ( uint8_t )g;
          } else {
            b = atoi( item );

            pixels[pixel_count * 3 + 2] = ( uint8_t )b;

            draw_seg_map_pixel( pixel_count );

            int color = 0;

            if ( seg_map[pixel_count] == 1 ) {
              color = color_space_max;
            }

            fprintf( file_write, "%d %d %d\n", color, color, color );

            pixel_count += 1;

            r = -1;
            g = -1;
            b = -1;
          }
        }

        item[0] = 0;
        item[1] = 0;
        item[2] = 0;
        item[3] = 0;
        item[4] = 0;

        item_count = 0;
      }
    }
  }

  fclose( file_read );
  fclose( file_write );
}

int main() {
  srand( time( NULL ) );

  double total_time = 0.0;

  pixels = ( uint8_t* ) malloc( width * height * 3 * sizeof( uint8_t ) );
  last_pixels = ( uint8_t* ) malloc( width * height * 3 * sizeof( uint8_t ) );
  seg_map = ( uint8_t* ) malloc( width * height * sizeof( uint8_t ) );
  samples = ( uint8_t** ) malloc( width * height * sizeof( uint8_t* ) );

  for ( int i = 0; i < width * height; i++ ) {
    samples[i] = ( uint8_t* ) malloc( sample_count * 3 * sizeof( uint8_t ) );
  }

  for ( int i = 0; i < total_frames; i++ ) {
    if ( i == 0 ) {
      char file_name[50];

      sprintf( file_name, "input%d.ppm", i );
      load_image( file_name );

      initialize_model();

      draw_seg_map();

      sprintf( file_name, "output/output%05d.ppm", i );
      save_seg_map( file_name );
    } else {
      clock_t start, end;
      start = clock();

      vibe_main( i );

      end = clock();
      total_time += ((double) (end - start)) / CLOCKS_PER_SEC;
    }
  }

  printf( "total_time: %f\n", total_time );
  printf( "total_frames: %d\n", total_frames );
  printf( "frames per second: %f\n", ( ( double )total_frames ) / total_time );

  free( pixels );
  free( last_pixels );
  free( seg_map );

  for ( int i = 0; i < width * height; i++ ) {
    free( samples[i] );
  }

  free( samples );
}
