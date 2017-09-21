FROM base/devel
RUN echo "[multilib]\nInclude = /etc/pacman.d/mirrorlist" >> /etc/pacman.conf
RUN pacman -Syy --noconfirm \
	staging/clang zlib boost git wget postgresql-libs && \
	rm -f \
      /var/cache/pacman/pkg/* \
      /var/lib/pacman/sync/* \
      /etc/pacman.d/mirrorlist.pacnew

RUN wget https://raw.githubusercontent.com/sbx320/binaries/master/makepkg -O /usr/bin/makepkg && \
	chmod +x /usr/bin/makepkg	
	  
RUN pacman -Syy --noconfirm && \
	git clone https://aur.archlinux.org/package-query.git && \
	cd package-query && \
	makepkg -si --noconfirm && \
	cd .. && \
	git clone https://aur.archlinux.org/yaourt.git && \
	cd yaourt && \
	makepkg -si --noconfirm && \
	cd .. && \
	rm -rf package-query yaourt && \
	rm -f \
      /var/cache/pacman/pkg/* \
      /var/lib/pacman/sync/* \
      /etc/pacman.d/mirrorlist.pacnew
		
RUN gpg --recv-keys 11E521D646982372EB577A1F8F0871F202119294 && \
	gpg --recv-keys B6C8F98282B944E3B0D5C2530FC3042E345AD05D && \
	yaourt -Syy --noconfirm	libc++ && \
	rm -f \
      /var/cache/pacman/pkg/* \
      /var/lib/pacman/sync/* \
      /etc/pacman.d/mirrorlist.pacnew
	  
# Setup compilers
ENV CXX="clang++ -fPIC -stdlib=libc++"
ENV CC="clang -fPIC"
ENV CPP="clang -E"
ENV LINK="clang++"

# Force clang 
RUN ln -sf /usr/bin/clang /usr/bin/cc && \
	ln -sf /usr/bin/clang++ /usr/bin/cpp
	
RUN yaourt -Syy --noconfirm \
	crypto++-git nlohmann_json-git premake-git  && \
	rm -f \
      /var/cache/pacman/pkg/* \
      /var/lib/pacman/sync/* \
      /etc/pacman.d/mirrorlist.pacnew

	  
COPY beast /usr/include/beast

# Protobuf 
COPY protobuf /mnt/protobuf
RUN cd /mnt/protobuf && \
	pacman -Syy && \
	makepkg -si --noconfirm && \
	rm -rf \
      /var/cache/pacman/pkg/* \
      /var/lib/pacman/sync/* \
      /etc/pacman.d/mirrorlist.pacnew \
	  /mnt/protobuf
