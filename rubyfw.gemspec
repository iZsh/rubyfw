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
Gem::Specification.new do |s|
  s.name = %q{rubyfw}
  s.version = "0.1.1"
  s.date = %q{2009-07-26}
  s.authors = ["iZsh"]
  s.email = %q{izsh@iphone-dev.com}
  s.homepage = %q{http://github.com/iZsh/rubyfw}
  s.summary = %q{FireWire Library for Ruby.}
  s.description = %q{FireWire Library for Ruby.}
  s.has_rdoc = false
  s.files = [
              "README", "ChangeLog", "LICENSE",
              "lib/fw.rb",
              "ext/extconf.rb", "ext/macosx/fwext.c"
            ]
  s.extensions = ["ext/extconf.rb"]
  
end
