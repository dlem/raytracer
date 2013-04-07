/**
 * This module exposes a function for recording (and possibly printing)
 * statistics about what we're doing.
 *
 * Name: Daniel Lemmond
 * User-id: dlemmond
 * Student id: 20302247
**/

#ifndef __STATS_HPP__
#define __STATS_HPP__

template<typename T>
void add_stat(const char *name, const T &t);

void dump_stats();

#endif
