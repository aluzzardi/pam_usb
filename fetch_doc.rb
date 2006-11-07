#!/usr/bin/env ruby

require 'open-uri'

BASE_URI = 'http://www.pamusb.org/doku/doc/'
DOC_PATH = './pam_usb/doc/'
DOCS = [ 'install', 'upgrading' ]

REPLACE_LIST = [
	# Extract text area from xhtml document.
	{ :pattern => /.*<textarea .+>(.*)<\/textarea>.*/m, :with => '\1' },

	# Remove wiki links [[link|name]]
	{ :pattern => /\[\[.+\|(.+)\]\]/, :with => '\1' },

	# Remove trailing whitespaces
	{ :pattern => /^  /, :with => '' },

	# Remove misc xhtml / wiki characters
	{ :pattern => /\/\//, :with => '' },
	{ :pattern => /\\\\ /, :with => '' },
	{ :pattern => /\*\*/, :with => '' },
	{ :pattern => /&lt;/, :with => '<' },
	{ :pattern => /&gt;/, :with => '>' },
	{ :pattern => /&quot;/, :with => '"' },
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
