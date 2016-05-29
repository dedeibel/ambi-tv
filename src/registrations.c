/* ambi-tv: a flexible ambilight clone for embedded linux
*  Copyright (C) 2013 Georg Kaindl
*  
*  This file is part of ambi-tv.
*  
*  ambi-tv is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*  
*  ambi-tv is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with ambi-tv.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "registrations.h"
#include "component.h"
#include "program.h"
#include "log.h"

#include "components/v4l2-grab-source.h"
#include "components/timer-source.h"
#include "components/avg-color-processor.h"
#include "components/edge-color-processor.h"
#include "components/mood-light-processor.h"
#include "components/lpd8806-spidev-sink.h"

#define LOGNAME      "registration: "

static const size_t REGISTRATION_SIZE = sizeof(struct ambitv_component_registration);

static struct ambitv_component_registration default_registrations[] = {
   {
      .name             = "v4l2-grab-source",
      .constructor      = (void* (*)(const char*, int, char**))ambitv_v4l2_grab_create
   },
   {
      .name             = "timer-source",
      .constructor      = (void* (*)(const char*, int, char**))ambitv_timer_source_create
   },
   {
      .name             = "avg-color-processor",
      .constructor      = (void* (*)(const char*, int, char**))ambitv_avg_color_processor_create
   },
   {
      .name             = "edge-color-processor",
      .constructor      = (void* (*)(const char*, int, char**))ambitv_edge_color_processor_create
   },
   {
      .name             = "mood-light-processor",
      .constructor      = (void* (*)(const char*, int, char**))ambitv_mood_light_processor_create
   },
   {
      .name             = "lpd8806-spidev-sink",
      .constructor      = (void* (*)(const char*, int, char**))ambitv_lpd8806_create
   },
   
   { NULL, NULL }
};
static struct ambitv_component_registration* registrations = default_registrations;

int
ambitv_register_component_for_name(const char* name, int argc, char** argv)
{
   int i, argv_copied = 0, ret = -1, aargc = argc;
   char** aargv = argv;
   struct ambitv_component_registration* r = registrations;
   
   while (NULL != r->name) {
      if (0 == strcmp(name, r->name)) {
         void* component;
         
         if (argc > 1) {
            for (i=1; i<argc; i+=2) {               
               if (0 == strcmp(argv[i], "--name")) {                  
                  name = argv[i+1];
                  
                  aargv = (char**)malloc(sizeof(char*) * (argc-1));
                  memcpy(aargv, argv, sizeof(char*) * i);
                  memcpy(&aargv[i], &argv[i+2], sizeof(char*) * (argc-i-1));
                  aargc = argc - 2;
                  
                  argv_copied = 1;
                  
                  break;
               }
            }
         }
         
         optind = 0;
         component = r->constructor(name, aargc, aargv);
         
         if (NULL != component) {
            
            if (ambitv_component_enable(component) < 0) {
               ambitv_log(ambitv_log_info, LOGNAME "failed to register component '%s' (class %s).\n",
                  name, r->name);
               ret = -4;
               goto returning;
            }
            
            ambitv_log(ambitv_log_info, LOGNAME "registered component '%s' (class %s).\n",
               name, r->name);
            
            ambitv_component_print_configuration(component);
            
            ret = 0;
            goto returning;
         } else {
            // TODO: error while constructing
            ret = -3;
            goto returning;
         }
      }
      
      r++;
   }
   
   ambitv_log(ambitv_log_error, LOGNAME "failed to find component class %s.\n",
      name);
   
returning:
   if (argv_copied) {
      free(aargv);
   }

   return ret;
}

int
ambitv_register_program_for_name(const char* name, int argc, char** argv)
{
   int ret = -1;
   
   struct ambitv_program* program = ambitv_program_create(name, argc, argv);
   if (NULL != program) {
      ret = ambitv_program_enable(program);
      
      if (ret < 0)
         ambitv_program_free(program);   
   }
   
   return ret;
}

size_t
ambitv_count_registration_entries(struct ambitv_component_registration* registrations) {
  size_t number_of_entries = 0;
  struct ambitv_component_registration* current;
  for (current = registrations; current->name != NULL; current++) {
    number_of_entries++;
  }
  return number_of_entries;
}

void
ambitv_append_component(struct ambitv_component_registration* component) {
  size_t number_of_entries = ambitv_count_registration_entries(registrations);
  struct ambitv_component_registration* new_registrations = (struct ambitv_component_registration*)malloc((number_of_entries + 2) * REGISTRATION_SIZE);

  struct ambitv_component_registration* current_old;
  struct ambitv_component_registration* current_new;
  // copy until terminator
  for (current_new = new_registrations, current_old = registrations; current_old->name != NULL; current_new++, current_old++) {
    memcpy(current_new, current_old, REGISTRATION_SIZE);
  }
  // insert the new component
  memcpy(current_new, component, REGISTRATION_SIZE);

  // append terminator
  current_new++;
  memcpy(current_new, current_old, REGISTRATION_SIZE);

  if (registrations != default_registrations) {
    free(registrations);
  }

  registrations = new_registrations;
}

char**
ambitv_get_component_list() {
  size_t number_of_entries = ambitv_count_registration_entries(registrations);
  char** components = (char**)malloc((number_of_entries + 1) * sizeof(char*));

  char** current_component = components;
  struct ambitv_component_registration* current_registration;
  for (current_component = components, current_registration = registrations; current_registration->name != NULL; current_registration++, current_component++) {
    *current_component = strdup(current_registration->name);
  }
  *current_component = NULL;
  return components;
}

void
ambitv_free_component_list(char** component_list) {
  char** current_component;
  for (current_component = component_list; *current_component != NULL; current_component++) {
    free(*current_component);
  }
  free(component_list);
}

