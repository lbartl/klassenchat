/* Copyright (C) 2015 Lukas Bartl
 * Diese Datei ist Teil des Klassenchats.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

///\file
// Bindet die Thread-Header <thread>, <mutex>, <condition_variable> und <atomic> ein

#ifndef THREAD_HPP
#define THREAD_HPP

#ifdef WIN32
# include "mingw-std-threads/mingw.thread.h"
# include <mutex>
# include "mingw-std-threads/mingw.mutex.h"
# include "mingw-std-threads/mingw.condition_variable.h" // bindet <atomic> ein
#else
# include <thread>
# include <condition_variable> // bindet <mutex> ein
# include <atomic>
#endif

namespace this_thread = std::this_thread;
using namespace std::literals; // Damit z.B. 0.5s, 50ms oder "baum"s geschrieben werden kann

using lock_guard = std::lock_guard <std::mutex> const; ///< meistens wird std::lock_guard mit std::mutex genutzt
using unique_lock = std::unique_lock <std::mutex>; ///< meistens wird std::unique_lock mit std::mutex genutzt

#endif // THREAD_HPP
