#!/usr/bin/env ruby
#
# fetch_doc.rb
#
# Fetch the documentation from the wiki and translate it
# to be human readable.
#

require 'open-uri'

BASE_URI = 'http://www.pamusb.org/wiki/doc/'
DOC_PATH = './pam_usb/doc/'
DOCS = [ 'install', 'upgrading', 'configuring' ]

REPLACE_LIST = [
	# Extract text area from xhtml document.
	{ :pattern => /.*<textarea .+>(.*)<\/textarea>.*/m, :with => '\1' },

	# Remove wiki links [[link|name]]
	{ :pattern => /\[\[.+\|(.+)\]\]/, :with => '\1' },

	# Remove misc xhtml/wiki characters
	{ :pattern => /\/\//, :with => '' },
	{ :pattern => /\\\\ /, :with => '' },
	{ :pattern => /\*\*/, :with => '' },
	{ :pattern => /&lt;/, :with => '<' },
	{ :pattern => /&gt;/, :with => '>' },
	{ :pattern => /&quot;/, :with => '"' },
	{ :pattern => /<code .+>.*\n/, :with => '' },
	{ :pattern => /<\/code>.*\n/, :with => '' },

	# Remove trailing whitespaces
	{ :pattern => /^  /, :with => '' },

	# Fit 80 colums
	{ :pattern => /(.{1,80})( +|$\n?)|(.{1,80})/, :with => "\\1\\3\n" },
]

def fetch_doc(name)
	uri = BASE_URI + name + '?do=edit'

	body = open(uri) { |f| f.read }
	REPLACE_LIST.each { |r| body.gsub!(r[:pattern], r[:with]) }

	body
end

DOCS.each do |doc|
	print "Fetching doc:#{doc}... "
	STDOUT.flush

	text = fetch_doc(doc)
	File.open(File.join(DOC_PATH, doc), 'w') { |f| f.write(text) }

	puts "Done."
end
