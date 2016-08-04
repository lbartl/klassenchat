/* Copyright (C) 2015,2016 Lukas Bartl
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
// Bindet die Thread-Header <thread>, <mutex>, <shared_mutex>, <condition_variable> und <atomic> ein

#ifndef THREAD_HPP
#define THREAD_HPP

#include <shared_mutex>

#ifdef WIN32
# include "mingw-threads/mingw.thread.h"
# include <boost/thread/shared_mutex.hpp>
# include <boost/thread/condition_variable.hpp>


using boost::mutex;
using boost::shared_mutex;
using boost::condition_variable;
using unique_lock = boost::unique_lock <mutex>;
#else
# include <thread>
# include <atomic>

using std::mutex;
using shared_mutex = std::shared_timed_mutex; ///< Unter Windows boost::shared_mutex, unter Unix std::shared_timed_mutex
using std::condition_variable;
using unique_lock = std::unique_lock <mutex>; ///< meistens wird std::unique_lock mit mutex genutzt
#endif

using lock_guard = std::lock_guard <mutex> const; ///< meistens wird std::lock_guard mit mutex genutzt
using shared_lock = std::shared_lock <shared_mutex>; ///< meistens wird std::shared_lock mit #shared_mutex genutzt

using namespace std::literals; // Damit z.B. 0.5s, 50ms oder "baum"s geschrieben werden kann
namespace this_thread = std::this_thread;
#endif // THREAD_HPP
