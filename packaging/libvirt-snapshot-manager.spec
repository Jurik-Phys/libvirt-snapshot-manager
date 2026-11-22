Name:           libvirt-snapshot-manager
Version:        0.3.2
Release:        1 

Summary:        Manage libvirt virtual machine external snapshots
License:        GPL-3.0-or-later
Group:          Other
URL:            https://github.com/jurik-phys/libvirt-snapshot-manager

Source0:	%name-%version.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  qt6-base-devel
BuildRequires:  qt6-svg-devel
BuildRequires:  qt6-svg

%description
A graphical lightweight tool for managing external snapshots
of virtual machines using libvirt and QEMU. It also offers
basic configuration capabilities for virtual machines,
including setting the guest operating system type,
allocated memory size, and CPU topology and model.

%prep
%setup

%build
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=%{_prefix}
cmake --build build --parallel %{?_smp_build_ncpus}

%install
DESTDIR=%{buildroot} cmake --install build

%files
%{_bindir}/libvirt-snapshot-manager
%{_datadir}/applications/libvirt-snapshot-manager.desktop
%{_datadir}/icons/hicolor/scalable/apps/libvirt-snapshot-manager.svg

%changelog

