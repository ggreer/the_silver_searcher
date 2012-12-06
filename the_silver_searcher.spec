Name:		the_silver_searcher
Version:	0.13.1
Release:	1%{?dist}
Summary:	A code-searching tool similar to ack, but faster

Group:		Applications/Utilities
License:	Apache v2.0
URL:		https://github.com/ggreer/%{name}
Source0:	https://github.com/downloads/ggreer/%{name}/%{name}-${version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	pkgconfig, autoconf, pcre-devel
Requires:	pcre

%description
The Silver Searcher
An attempt to make something better than ack (which itself is better than grep).

Why use Ag?
* It searches code about 3–5× faster than ack.
* It ignores file patterns from your .gitignore and .hgignore.
* If there are files in your source repo you don't want to search, just add their patterns to a .agignore file. *cough* extern *cough*
* The command name is 33% shorter than ack!

How is it so fast?
* Searching for literals (no regex) uses Boyer-Moore-Horspool strstr.
* Files are mmap()ed instead of read into a buffer.
* If you're building with PCRE 8.21 or greater, regex searches use the JIT compiler.
* Ag calls pcre_study() before executing the regex on a jillion files.
* Instead of calling fnmatch() on every pattern in your ignore files, non-regex patterns are loaded into an array and binary searched.
* Ag uses Pthreads to take advantage of multiple CPU cores and search files in parallel.

%prep
%setup -q


%build
aclocal
autoconf
autoheader
automake --add-missing
%configure 
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
rm -rf %{buildroot}/usr/share/doc

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_mandir}/*


%changelog
* Wed Dec 05 2012 Daniel Nelson <packetcollision@gmail.com> - 0.13.1-1
- Initial Build
