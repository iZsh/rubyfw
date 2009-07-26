# RubyFW: a Firewire Library for Ruby
# Copyright (c) 2009 iZsh - izsh at iphone-dev.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
require 'fwext'

# Expand the Bignum class for UI64 (Universal Identifier) representation
class Bignum
  def to_ui64
    ("%016x" % self).scan(/[a-f0-9]{2}/).join(":")
  end
end

module FW

  class FWDevice

    def to_s
      "#{guid.to_ui64} - #{vendorName}"
    end

    # (From the C Extension)
    # This class also contains the following methods:
    # guid() => Bignum
    # vendorName() => String
    # vendorID() => Fixnum
    # nodeID() => Fixnum
    # speed() => Fixnum
    # readQuadlet(Address) => { :value => Fixnum, :resultcode => Fixnum }
    # read(Address, Size) => { :buffer => String, :resultcode => Fixnum }
    # writeQuadlet(Address, Value) => Fixnum (result code)
    # write(Address, Buffer, Size) => Fixnum (result code)

  end

  class << self
    
    # (From the C Extension)
    # This Module also contains the following methods:
    # scanbus() => FWDevice Array
  
  end

end
