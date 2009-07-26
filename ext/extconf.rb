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
require 'mkmf'

@@extension_name = 'fwext'

def select_arch()
  case
#  when RUBY_PLATFORM =~ /win(dows|32)/i
#    TODO
#  when RUBY_PLATFORM =~ /linux/i
#    TODO
  when RUBY_PLATFORM =~ /darwin/i
    with_ldflags("-framework IOKit -framework CoreFoundation") {
      create_makefile(@@extension_name, "macosx")
    }
  else
    abort "#{RUBY_PLATFORM} platform not supported."
  end
  
end

select_arch()
