#!/usr/bin/env ruby
#
# fetch_doc.rb
#
# Fetch the documentation from the wiki and translate it
# to be human readable.
#

require 'open-uri'

BASE_URI = 'http://www.pamusb.org/doc/'
DOC_PATH = '../doc/'
DOCS = [ 'quickstart', 'upgrading', 'configuration', 'faq' ]
MANS = [ 'pamusb-check', 'pamusb-agent', 'pamusb-conf' ]

REPLACE_LIST = [
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
	{ :pattern => /<file>(.+)<\/file>/m, :with => '\1' },

	# Remove trailing whitespaces
	{ :pattern => /^  /, :with => '' },

	# Fit 80 colums
	{ :pattern => /(.{1,80})( +|$\n?)|(.{1,80})/, :with => "\\1\\3\n" },
]

def fetch_raw_doc(name)
	uri = BASE_URI + name + '?do=edit'

	body = open(uri) { |f| f.read }
	body.gsub!(/.*<textarea .+>\n(.*)<\/textarea>.*/m, '\1')
	body.gsub(/\r\n/, "\n")
end

def fetch_doc(name)
	body = fetch_raw_doc(name)
	REPLACE_LIST.each { |r| body.gsub!(r[:pattern], r[:with]) }

	body
end

Dir.chdir(File.dirname($0))

DOCS.each do |doc|
	print "Fetching doc:#{doc}... "
	STDOUT.flush

	text = fetch_doc(doc)
	File.open(File.join(DOC_PATH, doc.upcase), 'w') { |f| f.write(text) }

	puts "Done."
end

MANS.each do |man|
	print "Fetching man:#{man}... "
	STDOUT.flush

	doc = fetch_doc("man/#{man}")
	cmd = "txt2man -v \"PAM_USB\" -s1 -t#{man}"
	cmd += ' | sed "s/\\\\\\\\\\\\\\\\/\\\\\\/g"'
	File.popen("#{cmd} > #{File.join(DOC_PATH, man)}.1", 'w') do |f|
		f.write(doc)
	end
	begin
		File.unlink("#{File.join(DOC_PATH, man)}.1.gz")
	rescue Exception
	end
	system("gzip #{File.join(DOC_PATH, man)}.1")

	puts "Done."
end
