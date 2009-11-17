Summary: nss wrapper library that uses /etc/{passwd,shadow,group}.cache filenames
Name: libnss-cache
Version: 0.1
Release: 1
License: GPLv2
Group: System Environment/Base
Packager: Oliver Hookins <oliver.hookins@anchor.com.au>

URL: http://code.google.com/p/nsscache/
Source: http://nsscache.googlecode.com/files/%{name}-%{version}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArchitectures: i386, x86_64

%description
nsscache is a Python library and a commandline frontend to that library that
synchronises a local NSS cache against a remote directory service, such as
LDAP.

%prep
%setup -q -n %{name}

%build
make

%install
%{__rm} -rf %{buildroot}
make LIBDIR="%{buildroot}%{_libdir}" install

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%{_libdir}

%changelog
* Tue Jan 06 2009 Oliver Hookins <oliver.hookins@anchor.com.au> - 0.1-1
- Initial packaging
