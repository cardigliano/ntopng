/*
 *
 * (C) 2013-20 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef _MAC_HASH_H_
#define _MAC_HASH_H_

#include "ntop_includes.h"
 
class MacHash : public GenericHash {
 private:
  Mutex m;

 public:
  MacHash(NetworkInterface *iface, u_int _num_hashes, u_int _max_hash_size);

  Mac* get(const u_int8_t mac[6], bool is_inline_call);
};

#endif /* _MAC_HASH_H_ */
