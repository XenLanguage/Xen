Name:           Xen
%global debug_package %{nil}
Version:        0.5.3
Release:        1%{?dist}
Summary:        Xen

License:        ISC
URL:            https://jakerieger.github.io/Xen
Source0:        Xen-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  ninja-build
BuildRequires:  bash

%description
Xen programming language interpreter and tools.

%prep
%setup -q -n Xen-%{version}

%build
# Generate ninja build files
./generate_build.sh

# Build the release version for linux
cd build/linux-release
ninja

%install
rm -rf $RPM_BUILD_ROOT

# Install the main binary
install -D -m 0755 build/linux-release/bin/xen $RPM_BUILD_ROOT%{_bindir}/xen

# Install examples to /usr/share/xen
mkdir -p $RPM_BUILD_ROOT%{_datadir}/xen
cp -r examples $RPM_BUILD_ROOT%{_datadir}/xen/

%files
%license LICENSE
%doc README.md
%{_bindir}/xen
%{_datadir}/xen/examples/*

%changelog
* Thur Jan 08 2026 Jake Rieger <contact.jakerieger@gmail.com> - 0.5.3-1
- bug fixes and improvements
- new net namespace with TcpListener and TcpStream classes
