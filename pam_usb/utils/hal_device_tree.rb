#!/usr/bin/env ruby
#
# hal_device_tree.rb
#
# Print a tree from the output of hal-device
# Useful to analyse bug reports
#

require 'getopts'
require 'open-uri'

def parse_file(path)
	io = path
	io = open(io) if io.class != IO

	list = Array.new
	current = nil
	io.each_line do |line|
		if line =~ /^\d/
			list << current unless current.nil?
			current = Hash.new
			next
		end

		line.gsub!(/\(.*\)/, '')
		line.gsub!("'", '')
		k,v = line.split('=')
		next if k.strip.empty?
		current[k.strip] = v.strip
	end

	list
end

def print_node(node, pad = 0)
	print '|'
	pad.times { print "----" }
	print '-- '

	print "#{node['info.category']} " if node['info.category']
	print "#{node['info.product']} " if node['info.product']
	print " -> #{node['info.udi']}" if $OPT_u
	puts
end

def collect_children(list, root)
	root['childs'] = Array.new
	children = list.select { |e| e['info.parent'] == root['info.udi'] }
	children.each do |child|
		root['childs'] << child
		collect_children(list, child)
	end
end

def print_children(root, pad = 1)
	root['childs'].each do |child|
		print_node(child, pad)
		if not child['childs'].empty?
			print_children(child, pad + 1)
		end
	end
end

raise ArgumentError if getopts('uf:').nil?

list = parse_file($OPT_f || STDIN)
roots = list.select { |e| e['info.parent'].match(/computer/) unless e['info.parent'].nil? }

roots.each do |root|
	collect_children(list, root)
	print_node(root)
	print_children(root)
	puts '|'
end
