Summary: NSS module for directory services using an indexed, local disk cache
Name: libnss-cache
Version: 0.15
Release: 1
License: LGPLv3
Group: System Environment/Base
Packager: Oliver Hookins <oliver.hookins@anchor.com.au>

URL: https://github.com/google/libnss-cache/
Source: https://github.com/google/libnss-cache/archive/version/%{version}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch: i386, x86_64

%description
libnss-cache is a NSS module for reading directory service information for
hosts from an indexed, local disk cache of that directory service.  It is paired
with the nsscache python program that generates the cache from LDAP or other
directory services.

%prep
%setup -q -n %{name}-version-%{version}

%build
make

%install
%{__rm} -rf %{buildroot}
make LIBDIR="%{buildroot}%{_libdir}" install

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
%{_libdir}/*

%changelog
* Mon Mar 07 2016 Kevin Bowling <kbowling@freebsd.org> - 0.15-1
- Update to current release
- Correct license to LGPLv3
- Change URL and Source to GitHub, more accurate description for this package
* Tue Jan 06 2009 Oliver Hookins <oliver.hookins@anchor.com.au> - 0.1-1
- Initial packaging
